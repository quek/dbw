#include "Vst3Module.h"
#include <fstream>
#include <public.sdk/source/vst/hosting/module.h>
#include <public.sdk/source/vst/hosting/parameterchanges.h>
#include <pluginterfaces/base/funknown.h>
#include <pluginterfaces/vst/vsttypes.h>
#include <pluginterfaces/vst/ivsteditcontroller.h>
#include <pluginterfaces/vst/ivstevents.h>
#include <pluginterfaces/base/ftypes.h>
#include <public.sdk/source/common/memorystream.h>
#include "imgui.h"
#include "AudioEngine.h"
#include "Composer.h"
#include "Config.h"
#include "Error.h"
#include "logger.h"
#include "Project.h"
#include "Track.h"
#include "util.h"

// ここからVSTプラグイン関係(音声処理クラスやパラメーター操作クラス)のヘッダ

Vst3Module::Vst3Module(std::string name, Track* track) : Module(name, track), _pluginContext(this) {
}

Vst3Module::~Vst3Module() {
    closeGui();
    stop();
    if (_processor) {
        _processor->release();
    }
    if (_plugProvider) {
        _plugProvider->releasePlugIn(_component, _controller);
    }
}

bool Vst3Module::load(std::string path) {
    // ---------------------------------------------------------------------------
    // PluginContextを作成・セットアップする
    // PluginContextはVST3プラグインがホストアプリ名やインターフェイスの対応状況を取得したり、
    // allocateMessage()でIMessageを取得するために必要。(詳細は割愛)
    Steinberg::Vst::PluginContextFactory::instance().setPluginContext((&_pluginContext)->unknownCast());

    std::string error;
    _module = VST3::Hosting::Module::create(path, error);
    if (!_module) {
        Error("Vst3 create error {}", error);
        return false;
    }

    VST3::Hosting::PluginFactory factory = _module->getFactory();
    VST3::Hosting::FactoryInfo factoryInfo = factory.info();
    logger->debug("プラグイン情報");
    logger->debug("  Vendor : {}", factoryInfo.vendor().c_str());
    logger->debug("  URL    : {}", factoryInfo.url().c_str());
    logger->debug("  EMail  : {}", factoryInfo.email().c_str());

    // PluginFactoryクラスでVST3プラグインファイル内にあるクラスを列挙しながら
    // 音声処理クラスのみをaudioClassInfoにコピーする
    // VST3プラグインファイル内には複数の音声処理クラスが存在する場合があるため一度列挙する
    logger->debug("クラス情報");
    std::vector<VST3::Hosting::ClassInfo> audioClassInfo;
    for (VST3::Hosting::ClassInfo classInfo : factory.classInfos()) {
        logger->debug("  Name       : {}", classInfo.name().c_str());
        logger->debug("  UID        : {}", classInfo.ID().toString().c_str());
        logger->debug("  Category   : {}", classInfo.category().c_str());
        logger->debug("  Vendor     : {}", classInfo.vendor().c_str());
        logger->debug("  Version    : {}", classInfo.version().c_str());
        logger->debug("  VST SDK    : {}", classInfo.sdkVersion().c_str());
        logger->debug("  SubCategory: {}", classInfo.subCategoriesString().c_str());
        logger->debug("  Flags      : %d", classInfo.classFlags());
        // 音声処理クラスはカテゴリーがkVstAudioEffectClassとなっているものとなる
        // (ちなみにパラメータ操作クラスはkVstComponentControllerClass )
        if (classInfo.category() == kVstAudioEffectClass) {
            audioClassInfo.push_back(classInfo);
        }
    }

    // VST3プラグインファイル内に音声処理クラスがなかった場合は終了する。
    if (audioClassInfo.size() == 0) {
        logger->error("Error : VST3ファイル内に音声処理クラスがありません。");
        return false;
    }

    // ---------------------------------------------------------------------------
    // PlugProviderクラスを作成する
    // PlugProviderクラスはPluginFactoryクラスとClassInfoクラスから音声処理クラスを
    // 作成・初期化するクラス
    _plugProvider = std::make_unique<Steinberg::Vst::PlugProvider>(factory, audioClassInfo[0], true);
    if (!_plugProvider) {
        // 読み込みに失敗したら終了
        logger->error("Error : PlugProviderクラスの作成に失敗");
        return false;
    }
    _plugProvider->initialize();

    logger->debug("最初に見つかった音声処理クラス「{}」を作成・初期化しました。", audioClassInfo[0].name());
    _id = audioClassInfo[0].ID().toString();

    // ---------------------------------------------------------------------------
    // PlugProviderクラスから音声処理クラスを取得する。(音声処理クラスは初期化済み)
    _component = _plugProvider->getComponent();
    _controller = _plugProvider->getController();

    // https://steinbergmedia.github.io/vst3_dev_portal/pages/Technical+Documentation/API+Documentation/Index.html?highlight=setComponentState#initialization-of-communication-from-host-point-of-view
    Steinberg::MemoryStream processorStream;
    if (_component->getState(&processorStream) != Steinberg::kResultOk) {
        Error("{} のプロセスステータスの取得に失敗しました。", _name);
    }
    processorStream.seek(0, Steinberg::IBStream::IStreamSeekMode::kIBSeekSet, 0);
    Steinberg::tresult result = _controller->setComponentState(&processorStream);
    if (result != Steinberg::kResultOk && result != Steinberg::kNotImplemented) {
        Error("{} のステータス設定に失敗しました。", _name, result);
    }

    // 音声処理クラスはIComponentクラスとIAudioProcessorクラスを継承している
    // PlugProviderクラスから取得できるのはIComponentクラスのみなので、
    // IComponentクラス取得後、queryInterfaceでIAudioProcessorクラスも取得する
    _component->queryInterface(Steinberg::Vst::IAudioProcessor::iid, (void**)&_processor);
    if (_processor == nullptr) {
        logger->error("Error : 音声処理クラスの実装が不十分なため失敗");
        return false;
    }

    _controller->setComponentHandler(&_pluginContext);

    // ---------------------------------------------------------------------------
    // 取得した音声処理クラスのセットアップを行う。(その1)
    // 音声入出力を確認する。

    logger->debug("==================== 音声入出力の情報 ====================");

    // 音声入力情報の取得
    _ninputs = _component->getBusCount(Steinberg::Vst::MediaTypes::kAudio, Steinberg::Vst::BusDirections::kInput);
    logger->debug("音声入力バスは {} 個あります。", _ninputs);

    // 各音声入力バスの情報を取得する
    for (int i = 0; i < _ninputs; i++) {
        // バスの情報(名称・チャンネル数など)を取得する
        Steinberg::Vst::BusInfo busInfo;
        _component->getBusInfo(Steinberg::Vst::MediaTypes::kAudio, Steinberg::Vst::BusDirections::kInput, i, busInfo);

        // スピーカー構成のビットを取得する
        Steinberg::Vst::SpeakerArrangement arr = 0;
        _processor->getBusArrangement(Steinberg::Vst::BusDirections::kInput, i, arr);

        // logger->debug("  音声入力バス {}個目 … バス名: {}  チャンネル数(スピーカー構成フラグ)：{}ch", i + 1, busInfo.name, busInfo.channelCount/*, arr*/);
    }

    // 音声出力情報の取得(音声入力と同様の処理なので詳細なコメントは割愛)
    _noutputs = _component->getBusCount(Steinberg::Vst::MediaTypes::kAudio, Steinberg::Vst::BusDirections::kOutput);
    logger->debug("音声出力バスは {} 個あります。", _noutputs);

    for (int i = 0; i < _noutputs; i++) {
        Steinberg::Vst::BusInfo busInfo;
        _component->getBusInfo(Steinberg::Vst::MediaTypes::kAudio, Steinberg::Vst::BusDirections::kOutput, i, busInfo);

        Steinberg::Vst::SpeakerArrangement arr = 0;
        _processor->getBusArrangement(Steinberg::Vst::BusDirections::kOutput, i, arr);

        // logger->debug("  音声出力バス {}個目 … バス名: {}  チャンネル数(スピーカー構成フラグ)：{}ch", i + 1, busInfo.name, busInfo.channelCount/*, arr*/);
    }

    // ---------------------------------------------------------------------------
    // 取得した音声処理クラスのセットアップを行う。(その2)
    // イベント(MIDI)入出力を確認する。

    logger->debug("================== イベント入出力の情報 ==================");

    // イベント(MIDI等)入力情報の取得
    _neventInputs = _component->getBusCount(Steinberg::Vst::MediaTypes::kEvent, Steinberg::Vst::BusDirections::kInput);
    logger->debug("イベント入力バスは {} 個あります。", _neventInputs);

    // 各イベント入力バスの情報を取得する
    for (int i = 0; i < _neventInputs; i++) {
        // バスの情報(名称・チャンネル数など)を取得する
        Steinberg::Vst::BusInfo busInfo;
        _component->getBusInfo(Steinberg::Vst::MediaTypes::kEvent, Steinberg::Vst::BusDirections::kInput, i, busInfo);

        // logger->debug("  イベント入力バス {}個目 … バス名: {}  MIDIチャンネル数：{}ch", i + 1, busInfo.name, busInfo.channelCount);
    }

    // イベント(MIDI等)出力情報の取得(イベント入力と同様の処理なので詳細なコメントは割愛)
    _neventOutputs = _component->getBusCount(Steinberg::Vst::MediaTypes::kEvent, Steinberg::Vst::BusDirections::kOutput);
    logger->debug("イベント出力バスは {} 個あります。", _neventOutputs);

    for (int i = 0; i < _neventOutputs; i++) {
        Steinberg::Vst::BusInfo busInfo;
        _component->getBusInfo(Steinberg::Vst::MediaTypes::kEvent, Steinberg::Vst::BusDirections::kInput, i, busInfo);

        // logger->debug("  イベント出力バス {}目 … バス名: {}  MIDIチャンネル数：{}ch", i + 1, busInfo.name, busInfo.channelCount);
    }

    // ---------------------------------------------------------------------------
    // 取得した音声処理クラスのセットアップを行う。(その3)
    // 対応しているビット数・レイテンシなどを確認する。

    logger->debug("=================== その他 音声処理情報 ==================");

    // 音声のサンプリングビット数
    logger->debug("音声入出力の対応ビット数");

    Steinberg::tresult resSampleBit32 = _processor->canProcessSampleSize(Steinberg::Vst::SymbolicSampleSizes::kSample32);
    if (resSampleBit32 == Steinberg::kResultTrue) {
        _symbolicSampleSizes = Steinberg::Vst::SymbolicSampleSizes::kSample32;
    }
    Steinberg::tresult resSampleBit64 = _processor->canProcessSampleSize(Steinberg::Vst::SymbolicSampleSizes::kSample64);
    if (resSampleBit64 == Steinberg::kResultTrue) {
        _symbolicSampleSizes = Steinberg::Vst::SymbolicSampleSizes::kSample64;
    }

    logger->debug("　32bit対応 … {}  64bit対応 … {}", (resSampleBit32 != Steinberg::kResultFalse) ? "対応" : "非対応", (resSampleBit64 != Steinberg::kResultFalse) ? "対応" : "非対応");

    // ---------------------------------------------------------------------------
    // 取得した音声処理クラスのセットアップを行う。(その4)
    // 音声処理するためにセットアップする。

    logger->debug("============== 音声処理クラスのセットアップ ==============");

    // セットアップのために音声処理クラスを非アクティブにする
    _processor->setProcessing(false);
    _component->setActive(false);

    // サンプリングレート、ブロックサイズ等をProcessSetup構造体に設定。
    // プラグインが対応しているサンプリングレート・ブロックサイズは取得できないので、
    // VSTホスト側でサンプリングレート・ブロックサイズを適当に設定して、
    // setupProcessing()でkResultOkとなるパターンを探す必要がある。
    Steinberg::Vst::SampleRate sampleRate = 48000.0;
    Steinberg::int32 blockSize = 1024;
    Steinberg::Vst::ProcessSetup setup{ Steinberg::Vst::ProcessModes::kRealtime, _symbolicSampleSizes, blockSize, sampleRate };

    if (_processor->setupProcessing(setup) != Steinberg::kResultOk) {
        logger->debug("Error : 音声処理クラスのセットアップに失敗");
        logger->debug("      : 32bit、サンプリングレート {}Hz、ブロックサイズ {}に非対応", sampleRate, blockSize);
    } else {
        logger->debug("音声処理クラスのセットアップが完了");
        logger->debug("プラグインは32bit、サンプリングレート {}Hz、ブロックサイズ {}に対応", sampleRate, blockSize);
    }

    for (auto index = 0; index < _ninputs; ++index) {
        _component->activateBus(Steinberg::Vst::MediaTypes::kAudio,
                                Steinberg::Vst::BusDirections::kInput,
                                index, true);
    }
    for (auto index = 0; index < _noutputs; ++index) {
        _component->activateBus(Steinberg::Vst::MediaTypes::kAudio,
                                Steinberg::Vst::BusDirections::kOutput,
                                index, true);
    }
    for (auto index = 0; index < _neventInputs; ++index) {
        _component->activateBus(Steinberg::Vst::MediaTypes::kEvent,
                                Steinberg::Vst::BusDirections::kInput,
                                index, true);
    }
    for (auto index = 0; index < _neventOutputs; ++index) {
        _component->activateBus(Steinberg::Vst::MediaTypes::kEvent,
                                Steinberg::Vst::BusDirections::kOutput,
                                index, true);
    }

    return true;
}

bool Vst3Module::process(ProcessBuffer* buffer, int64_t steadyTime) {
    Steinberg::Vst::ProcessData processData;
    ///< processing mode - value of \ref ProcessModes
    processData.processMode = Steinberg::Vst::ProcessModes::kRealtime;
    ///< sample size - value of \ref SymbolicSampleSizes
    processData.symbolicSampleSize = _symbolicSampleSizes;
    ///< number of samples to process
    processData.numSamples = buffer->_framesPerBuffer;
    ///< number of audio input busses
    processData.numInputs = _ninputs;
    ///< number of audio output busses
    processData.numOutputs = _noutputs;
    ///< buffers of input busses
    // TODO track->addModule(module) のときにやる
    std::vector<Steinberg::Vst::AudioBusBuffers> inputAudioBusBuffers;
    std::vector<Steinberg::Vst::AudioBusBuffers> outputAudioBusBuffers;
    std::vector<std::vector<float*>> inputChannelBuffers32;
    std::vector<std::vector<double*>> inputChannelBuffers64;
    std::vector<std::vector<float*>> outputChannelBuffers32;
    std::vector<std::vector<double*>> outputChannelBuffers64;
    for (int i = 0; i < nbuses(); ++i) {
        if (_symbolicSampleSizes == Steinberg::Vst::SymbolicSampleSizes::kSample32) {
            buffer->ensure32();
            std::vector<float*> inChannels;
            for (auto& x : buffer->_in.at(i).buffer32()) {
                inChannels.push_back(x.data());
            }
            inputChannelBuffers32.push_back(inChannels);
            std::vector<float*> outChannels;
            for (auto& x : buffer->_out.at(i).buffer32()) {
                outChannels.push_back(x.data());
            }
            outputChannelBuffers32.push_back(outChannels);
        } else {
            buffer->ensure64();
            std::vector<double*> inChannels;
            for (auto& x : buffer->_in.at(i).buffer64()) {
                inChannels.push_back(x.data());
            }
            inputChannelBuffers64.push_back(inChannels);
            std::vector<double*> outChannels;
            for (auto& x : buffer->_out.at(i).buffer64()) {
                outChannels.push_back(x.data());
            }
            outputChannelBuffers64.push_back(outChannels);
        }
        Steinberg::Vst::AudioBusBuffers inBus;
        inBus.numChannels = buffer->_nchannels;
        if (_symbolicSampleSizes == Steinberg::Vst::SymbolicSampleSizes::kSample32) {
            inBus.channelBuffers32 = inputChannelBuffers32.back().data();
        } else {
            inBus.channelBuffers64 = inputChannelBuffers64.back().data();
        }
        inputAudioBusBuffers.emplace_back(inBus);

        Steinberg::Vst::AudioBusBuffers outBus;
        outBus.numChannels = buffer->_nchannels;
        if (_symbolicSampleSizes == Steinberg::Vst::SymbolicSampleSizes::kSample32) {
            outBus.channelBuffers32 = outputChannelBuffers32.back().data();
        } else {
            outBus.channelBuffers64 = outputChannelBuffers64.back().data();
        }
        outputAudioBusBuffers.emplace_back(outBus);
    }
    processData.inputs = inputAudioBusBuffers.data();
    processData.outputs = outputAudioBusBuffers.data();

    ///< incoming parameter changes for this block
    Steinberg::Vst::ParameterChanges inputParams;
    processData.inputParameterChanges = &inputParams;
    ///< outgoing parameter changes for this block (optional)
    Steinberg::Vst::ParameterChanges outputParams;
    processData.outputParameterChanges = &outputParams;
    ///< incoming events for this block (optional)
    Steinberg::Vst::EventList inputEventList = buffer->_eventIn.vst3InputEvents();
    processData.inputEvents = (Steinberg::Vst::IEventList*)&inputEventList;
    ///< outgoing events for this block (optional)
    Steinberg::Vst::EventList outputEventList = buffer->_eventIn.vst3OutputEvents();
    processData.outputEvents = (Steinberg::Vst::IEventList*)&outputEventList;

    ///< processing context (optional, but most welcome)
    Steinberg::uint32 statesAndFlangs = 0;
    if (_track->_composer->_playing) {
        statesAndFlangs |= Steinberg::Vst::ProcessContext::StatesAndFlags::kPlaying;
    }
    statesAndFlangs |= Steinberg::Vst::ProcessContext::StatesAndFlags::kTempoValid;
    statesAndFlangs |= Steinberg::Vst::ProcessContext::StatesAndFlags::kProjectTimeMusicValid;
    statesAndFlangs |= Steinberg::Vst::ProcessContext::StatesAndFlags::kBarPositionValid;
    Steinberg::Vst::ProcessContext processContext = {};
    processContext.state = statesAndFlangs;
    double sampleRate = gPreference.sampleRate;
    processContext.sampleRate = sampleRate;
    double playTime = _track->_composer->_playTime;
    processContext.projectTimeMusic = playTime;
    processContext.barPositionMusic = static_cast<int>(playTime / 4);
    double bpm = _track->_composer->_bpm;
    // TODO これあってる？
    processContext.projectTimeSamples = playTime / (bpm / 60.0) * sampleRate;
    processContext.tempo = bpm;
    processContext.timeSigNumerator = 4;
    processContext.timeSigDenominator = 4;

    processData.processContext = &processContext;

    _processor->process(processData);

    for (int bus = 0; bus < _noutputs; ++bus) {
        for (int channel = 0; channel < processData.outputs[bus].numChannels; ++channel) {
            buffer->_out[bus]._constantp[channel] =
                (processData.outputs[bus].silenceFlags & (static_cast<unsigned long long>(1) << channel)) != 0;
        }
    }

    return Module::process(buffer, steadyTime);
}

void Vst3Module::start() {
    if (_component) {
        _component->setActive(true);
    }
    if (_processor) {
        _latency = _processor->getLatencySamples();
        _track->_composer->computeLatency();

        Steinberg::uint32 tailSample = _processor->getTailSamples();
        if (tailSample != Steinberg::Vst::kNoTail) {
            logger->debug("プラグインのテイルサンプル … {}", tailSample);
        }

        _processor->setProcessing(true);
    }
    Module::start();
}

void Vst3Module::stop() {
    Module::stop();
    if (_processor) {
        _processor->setProcessing(false);
    }
    if (_component) {
        _component->setActive(false);
    }
}

void Vst3Module::openGui() {
    _plugView = _controller->createView(Steinberg::Vst::ViewType::kEditor);
    if (_plugView->isPlatformTypeSupported(Steinberg::kPlatformTypeHWND) != Steinberg::kResultOk) {
        logger->debug("VST3 plugin does not support HWND.");
        return;
    }

    _plugView->setFrame(&_pluginContext);

    Steinberg::ViewRect size;
    _plugView->getSize(&size);

    bool resizable = _plugView->canResize() == Steinberg::kResultTrue;
    _editorWindow = std::make_unique<PluginEditorWindow>(this, size.getWidth(), size.getHeight(), resizable);
    if (_plugView->attached(_editorWindow->_hwnd, Steinberg::kPlatformTypeHWND) != Steinberg::kResultOk) {
        logger->debug("VST3 attached failed!");
        _editorWindow.reset();
        return;
    }

    Module::openGui();
}

void Vst3Module::closeGui() {
    if (_plugView != nullptr) {
        _plugView->removed();
        _plugView->release();
        _plugView = nullptr;
    }
    if (_editorWindow != nullptr) {
        _editorWindow.reset();
    }
    Module::closeGui();
}

void Vst3Module::renderContent() {
    if (_didOpenGui) {
        if (ImGui::Button("Close")) {
            closeGui();
        }
    } else {
        if (ImGui::Button("Open")) {
            openGui();
        }
    }
}

void Vst3Module::onResize(int width, int height) {
    if (_plugView) {
        Steinberg::ViewRect r{};
        r.right = width;
        r.bottom = height;
        Steinberg::ViewRect r2{};
        if (_plugView->getSize(&r2) == Steinberg::kResultTrue &&
            (r.getWidth() != r2.getWidth() || r.getHeight() != r2.getHeight())) {
            _plugView->onSize(&r);
        }
    }
}

void Vst3Module::loadState(std::filesystem::path path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        Error("{} のステートファイルをオープンできませんでした。{}", _name, path.string());
    }

    int64_t size = 0;
    in.read((char*)&size, sizeof(int64_t));
    if (!in) {
        Error("{} のステートサイズを取得できませんでした。{}", _name, path.string());
    }

    std::unique_ptr<char[]> buffer(new char[size]);
    in.read(buffer.get(), size);
    if (!in) {
        Error("{}のステートを読み込めませんでした。{}", _name, path.string());
    }
    auto readSize = in.gcount();
    if (readSize != size) {
        Error("{}のステートの読み込みサイズ不正。{}/{} {}", _name, std::to_string(readSize), std::to_string(size), path.string());
    }

    Steinberg::MemoryStream processorStream(buffer.get(), size);
    Steinberg::tresult result = _component->setState(&processorStream);
    if (result != Steinberg::kResultOk) {
        Error("{}のステータス設定に失敗しました。{}", _name, std::to_string(result));
    }
    processorStream.seek(0, Steinberg::IBStream::IStreamSeekMode::kIBSeekSet, 0);
    result = _controller->setComponentState(&processorStream);
    if (result != Steinberg::kResultOk && result != Steinberg::kNotImplemented) {
        Error("{}のステータス設定に失敗しました。{}", _name, std::to_string(result));
    }

    in.read((char*)&size, sizeof(int64_t));
    if (!in) {
        Error("{}のコントロールステートサイズを取得できませんでした。{}", _name, path.string());
    }

    buffer.reset(new char[size]);
    in.read(buffer.get(), size);
    if (!in) {
        Error("{}のコントロールステートを読み込めませんでした。{}", _name, path.string());
    }
    readSize = in.gcount();
    if (readSize != size) {
        Error("{}のコントロールステートの読み込みサイズ不正。{}/{} {}", _name, std::to_string(readSize), std::to_string(size), path.string());
    }

    Steinberg::MemoryStream controllerStream(buffer.get(), size);
    result = _controller->setState(&controllerStream);
    if (result != Steinberg::kResultOk && result != Steinberg::kNotImplemented) {
        Error("{}のステータス設定に失敗しました。{}", _name, result);
    }
}

tinyxml2::XMLElement* Vst3Module::toXml(tinyxml2::XMLDocument* doc) {
    Steinberg::MemoryStream processorStream;
    if (_component->getState(&processorStream) != Steinberg::kResultOk) {
        Error("{} のプロセスステータスの取得に失敗しました。", _name);
    }
    Steinberg::MemoryStream controllerStream;
    auto result = _controller->getState(&controllerStream);
    if (result != Steinberg::kResultOk && result != Steinberg::kNotImplemented) {
        Error("{}のコントロールステータスの取得に失敗しました。{}", _name, result);
    }

    auto* element = doc->NewElement("Vst3Plugin");
    element->SetAttribute("id", xmlId());
    // TODO Possible values: instrument, noteFX, audioFX, analyzer
    element->SetAttribute("deviceRole", "instrument");
    element->SetAttribute("deviceName", _name.c_str());
    element->SetAttribute("deviceID", _id.c_str());

    element->InsertNewChildElement("Parameters");

    auto* enabled = element->InsertNewChildElement("Enabled");
    enabled->SetAttribute("value", true);

    auto* state = element->InsertNewChildElement("State");
    auto statePath = std::filesystem::path("plugins") / (generateUniqueId() + ".vstpreset");
    state->SetAttribute("path", statePath.string().c_str());

    auto path = _track->_composer->_project->projectDir() / statePath;
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        Error("{}のステータス保存に失敗しました。", _name);
    }
    int64_t size = processorStream.getSize();
    out.write((const char*)&size, sizeof(int64_t));
    if (!out) {
        Error("{}のステータス保存に失敗しました。", _name);
    }
    out.write(processorStream.getData(), size);
    if (!out) {
        Error("{}のステータス保存に失敗しました。", _name);
    }
    size = controllerStream.getSize();
    out.write((const char*)&size, sizeof(int64_t));
    if (!out) {
        Error("{}のステータス保存に失敗しました。", _name);
    }
    out.write(controllerStream.getData(), size);
    if (!out) {
        Error("{}のステータス保存に失敗しました。", _name);
    }
    out.close();
    if (out.fail()) {
        Error("{}のステータス保存に失敗しました。", _name);
    }

    auto connectionsElement = Module::toXml(doc);
    element->InsertEndChild(connectionsElement);

    return element;
}

nlohmann::json Vst3Module::scan(const std::string path) {
    nlohmann::json plugins;
    plugins["path"] = path;

    std::string error;
    auto module = VST3::Hosting::Module::create(path, error);
    if (!module) {
        Error("Vst3 create error {}", error);
        return false;
    }

    VST3::Hosting::PluginFactory factory = module->getFactory();
    VST3::Hosting::FactoryInfo factoryInfo = factory.info();
    plugins["Factory Info"]["Vendor"] = factoryInfo.vendor();
    plugins["Factory Info"]["URL"] = factoryInfo.url();
    plugins["Factory Info"]["E-Mail"] = factoryInfo.email();
    plugins["Factory Info"]["Flags"]["Unicode"] =
        (factoryInfo.flags() & Steinberg::PFactoryInfo::FactoryFlags::kUnicode) != 0;
    plugins["Factory Info"]["Flags"]["Classes Discardable"] = factoryInfo.classesDiscardable();
    plugins["Factory Info"]["Flags"]["Component Non Discardable"] = factoryInfo.componentNonDiscardable();

    std::vector<VST3::Hosting::ClassInfo> audioClassInfo;
    for (VST3::Hosting::ClassInfo classInfo : factory.classInfos()) {
        if (classInfo.category() == "Audio Module Class") {
            plugins["name"] = classInfo.name();
            plugins["id"] = classInfo.ID().toString();
        }
        nlohmann::json json;
        json["CID"] = classInfo.ID().toString();
        json["Category"] = classInfo.category();
        json["Name"] = classInfo.name();
        json["Vendor"] = classInfo.vendor();
        json["Version"] = classInfo.version();
        json["SDKVersion"] = classInfo.sdkVersion();
        auto& subCategories = classInfo.subCategories();
        for (auto& subCategory : subCategories) {
            json["Sub Categories"].push_back(subCategory);
        }
        json["Class Flags"] = classInfo.classFlags();

        plugins["Classes"].push_back(json);
    }

    return plugins;
}

