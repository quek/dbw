#include "Vst3Module.h"
#include <filesystem>
#include <fstream>
#include <public.sdk/source/vst/hosting/module.h>
#include <public.sdk/source/vst/hosting/parameterchanges.h>
#include <pluginterfaces/base/funknown.h>
#include <pluginterfaces/vst/vsttypes.h>
#include <pluginterfaces/vst/ivsteditcontroller.h>
#include <pluginterfaces/vst/ivstevents.h>
#include <pluginterfaces/base/ftypes.h>
#include <public.sdk/source/common/memorystream.h>
#include <public.sdk/source/vst/utility/stringconvert.h>
#include <public.sdk/source/vst/vstpresetfile.h>
#include "imgui.h"
#include <imgui-knobs/imgui-knobs.h>
#include <cppcodec/base64_rfc4648.hpp>
#include "App.h"
#include "AudioEngine.h"
#include "AutomationTarget.h"
#include "Composer.h"
#include "Config.h"
#include "Error.h"
#include "logger.h"
#include "Project.h"
#include "Track.h"
#include "util.h"
#include "command/ComputeLatency.h"

Vst3Module::Vst3Module(const nlohmann::json& json, SerializeContext& context) : Module(json, context), _pluginContext(this)
{
    std::ifstream in(configDir() / "plugin.json");
    if (!in)
    {
        return;
    }
    nlohmann::json plugins;
    in >> plugins;
    _id = json["_id"];
    auto plugin = std::find_if(plugins["vst3"].begin(), plugins["vst3"].end(), [this](auto x) { return x["id"] == _id; });
    if (plugin == plugins["vst3"].end())
    {
        return;
    }

    auto path = (*plugin)["path"].get<std::string>();
    load(path);
    std::string state = json["state"].get<std::string>();
    std::vector<uint8_t> buffer = cppcodec::base64_rfc4648::decode(state);
    Steinberg::MemoryStream stream(buffer.data(), buffer.size());
    if (!Steinberg::Vst::PresetFile::loadPreset(&stream, _component->iid, _component, _controller))
    {
        Error("Steinberg::Vst::PresetFile::loadPreset FAILED!");
    }
}

Vst3Module::Vst3Module(std::string name, Track* track) : Module(name, track), _pluginContext(this)
{
}

Vst3Module::~Vst3Module()
{
    closeGui();
    stop();
    if (_processor)
    {
        _processor->release();
        _processor = nullptr;
    }
    if (_plugProvider)
    {
        _plugProvider->releasePlugIn(_component, _controller);
        _component = nullptr;
        _controller = nullptr;
    }
}

bool Vst3Module::load(std::string path)
{
    // ---------------------------------------------------------------------------
    // PluginContextを作成・セットアップする
    // PluginContextはVST3プラグインがホストアプリ名やインターフェイスの対応状況を取得したり、
    // allocateMessage()でIMessageを取得するために必要。(詳細は割愛)
    Steinberg::Vst::PluginContextFactory::instance().setPluginContext((&_pluginContext)->unknownCast());

    std::string error;
    _module = VST3::Hosting::Module::create(path, error);
    if (!_module)
    {
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
    for (VST3::Hosting::ClassInfo classInfo : factory.classInfos())
    {
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
        if (classInfo.category() == kVstAudioEffectClass)
        {
            audioClassInfo.push_back(classInfo);
        }
    }

    // VST3プラグインファイル内に音声処理クラスがなかった場合は終了する。
    if (audioClassInfo.size() == 0)
    {
        logger->error("Error : VST3ファイル内に音声処理クラスがありません。");
        return false;
    }

    // ---------------------------------------------------------------------------
    // PlugProviderクラスを作成する
    // PlugProviderクラスはPluginFactoryクラスとClassInfoクラスから音声処理クラスを
    // 作成・初期化するクラス
    _plugProvider = std::make_unique<Steinberg::Vst::PlugProvider>(factory, audioClassInfo[0], true);
    if (!_plugProvider)
    {
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
    if (_component->getState(&processorStream) != Steinberg::kResultOk)
    {
        Error("{} のプロセスステータスの取得に失敗しました。", _name);
    }
    processorStream.seek(0, Steinberg::IBStream::IStreamSeekMode::kIBSeekSet, 0);
    Steinberg::tresult result = _controller->setComponentState(&processorStream);
    if (result != Steinberg::kResultOk && result != Steinberg::kNotImplemented)
    {
        Error("{} のステータス設定に失敗しました。", _name, result);
    }

    // 音声処理クラスはIComponentクラスとIAudioProcessorクラスを継承している
    // PlugProviderクラスから取得できるのはIComponentクラスのみなので、
    // IComponentクラス取得後、queryInterfaceでIAudioProcessorクラスも取得する
    _component->queryInterface(Steinberg::Vst::IAudioProcessor::iid, (void**)&_processor);
    if (_processor == nullptr)
    {
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
    for (int i = 0; i < _ninputs; i++)
    {
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

    for (int i = 0; i < _noutputs; i++)
    {
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
    for (int i = 0; i < _neventInputs; i++)
    {
        // バスの情報(名称・チャンネル数など)を取得する
        Steinberg::Vst::BusInfo busInfo;
        _component->getBusInfo(Steinberg::Vst::MediaTypes::kEvent, Steinberg::Vst::BusDirections::kInput, i, busInfo);

        // logger->debug("  イベント入力バス {}個目 … バス名: {}  MIDIチャンネル数：{}ch", i + 1, busInfo.name, busInfo.channelCount);
    }

    // イベント(MIDI等)出力情報の取得(イベント入力と同様の処理なので詳細なコメントは割愛)
    _neventOutputs = _component->getBusCount(Steinberg::Vst::MediaTypes::kEvent, Steinberg::Vst::BusDirections::kOutput);
    logger->debug("イベント出力バスは {} 個あります。", _neventOutputs);

    for (int i = 0; i < _neventOutputs; i++)
    {
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
    if (resSampleBit32 == Steinberg::kResultTrue)
    {
        _symbolicSampleSizes = Steinberg::Vst::SymbolicSampleSizes::kSample32;
    }
    Steinberg::tresult resSampleBit64 = _processor->canProcessSampleSize(Steinberg::Vst::SymbolicSampleSizes::kSample64);
    if (resSampleBit64 == Steinberg::kResultTrue)
    {
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

    if (_processor->setupProcessing(setup) != Steinberg::kResultOk)
    {
        logger->debug("Error : 音声処理クラスのセットアップに失敗");
        logger->debug("      : 32bit、サンプリングレート {}Hz、ブロックサイズ {}に非対応", sampleRate, blockSize);
    }
    else
    {
        logger->debug("音声処理クラスのセットアップが完了");
        logger->debug("プラグインは32bit、サンプリングレート {}Hz、ブロックサイズ {}に対応", sampleRate, blockSize);
    }

    for (auto index = 0; index < _ninputs; ++index)
    {
        _component->activateBus(Steinberg::Vst::MediaTypes::kAudio,
                                Steinberg::Vst::BusDirections::kInput,
                                index, true);
    }
    for (auto index = 0; index < _noutputs; ++index)
    {
        _component->activateBus(Steinberg::Vst::MediaTypes::kAudio,
                                Steinberg::Vst::BusDirections::kOutput,
                                index, true);
    }
    for (auto index = 0; index < _neventInputs; ++index)
    {
        _component->activateBus(Steinberg::Vst::MediaTypes::kEvent,
                                Steinberg::Vst::BusDirections::kInput,
                                index, true);
    }
    for (auto index = 0; index < _neventOutputs; ++index)
    {
        _component->activateBus(Steinberg::Vst::MediaTypes::kEvent,
                                Steinberg::Vst::BusDirections::kOutput,
                                index, true);
    }

    prepareParameterInfo();

    // 先頭の3つを初期表示に
    for (Steinberg::int32 i = 0; i < min(3, _controller->getParameterCount() - 1); ++i)
    {
        Steinberg::Vst::ParameterInfo parameterInfo = {};
        _controller->getParameterInfo(i, parameterInfo);
        updateEditedParamIdList(parameterInfo.id);
    }

    return true;
}

bool Vst3Module::process(ProcessBuffer* buffer, int64_t steadyTime)
{
    std::lock_guard<std::recursive_mutex> lock(_parameterChangesMtx);

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
    for (int i = 0; i < nbuses(); ++i)
    {
        if (_symbolicSampleSizes == Steinberg::Vst::SymbolicSampleSizes::kSample32)
        {
            buffer->ensure32();
            std::vector<float*> inChannels;
            for (auto& x : buffer->_in.at(i).buffer32())
            {
                inChannels.push_back(x.data());
            }
            inputChannelBuffers32.push_back(inChannels);
            std::vector<float*> outChannels;
            for (auto& x : buffer->_out.at(i).buffer32())
            {
                outChannels.push_back(x.data());
            }
            outputChannelBuffers32.push_back(outChannels);
        }
        else
        {
            buffer->ensure64();
            std::vector<double*> inChannels;
            for (auto& x : buffer->_in.at(i).buffer64())
            {
                inChannels.push_back(x.data());
            }
            inputChannelBuffers64.push_back(inChannels);
            std::vector<double*> outChannels;
            for (auto& x : buffer->_out.at(i).buffer64())
            {
                outChannels.push_back(x.data());
            }
            outputChannelBuffers64.push_back(outChannels);
        }
        Steinberg::Vst::AudioBusBuffers inBus;
        inBus.numChannels = buffer->_nchannels;
        if (_symbolicSampleSizes == Steinberg::Vst::SymbolicSampleSizes::kSample32)
        {
            inBus.channelBuffers32 = inputChannelBuffers32.back().data();
        }
        else
        {
            inBus.channelBuffers64 = inputChannelBuffers64.back().data();
        }
        inputAudioBusBuffers.emplace_back(inBus);

        Steinberg::Vst::AudioBusBuffers outBus;
        outBus.numChannels = buffer->_nchannels;
        if (_symbolicSampleSizes == Steinberg::Vst::SymbolicSampleSizes::kSample32)
        {
            outBus.channelBuffers32 = outputChannelBuffers32.back().data();
        }
        else
        {
            outBus.channelBuffers64 = outputChannelBuffers64.back().data();
        }
        outputAudioBusBuffers.emplace_back(outBus);
    }
    processData.inputs = inputAudioBusBuffers.data();
    processData.outputs = outputAudioBusBuffers.data();

    ///< incoming parameter changes for this block
    processData.inputParameterChanges = &_parameterChanges;
    ///< outgoing parameter changes for this block (optional)
    Steinberg::Vst::ParameterChanges outputParams;
    processData.outputParameterChanges = &outputParams;
    ///< incoming events for this block (optional)
    Steinberg::Vst::EventList inputEventList = buffer->_eventIn.vst3InputEvents(_noteOnKeys);
    processData.inputEvents = (Steinberg::Vst::IEventList*)&inputEventList;
    ///< outgoing events for this block (optional)
    Steinberg::Vst::EventList outputEventList = buffer->_eventIn.vst3OutputEvents();
    processData.outputEvents = (Steinberg::Vst::IEventList*)&outputEventList;

    ///< processing context (optional, but most welcome)
    Steinberg::uint32 statesAndFlangs = 0;
    if (_track->getComposer()->_playing)
    {
        statesAndFlangs |= Steinberg::Vst::ProcessContext::StatesAndFlags::kPlaying;
    }
    statesAndFlangs |= Steinberg::Vst::ProcessContext::StatesAndFlags::kTempoValid;
    statesAndFlangs |= Steinberg::Vst::ProcessContext::StatesAndFlags::kProjectTimeMusicValid;
    statesAndFlangs |= Steinberg::Vst::ProcessContext::StatesAndFlags::kBarPositionValid;
    Steinberg::Vst::ProcessContext processContext = {};
    processContext.state = statesAndFlangs;
    double sampleRate = gPreference.sampleRate;
    processContext.sampleRate = sampleRate;
    double playTime = _track->getComposer()->_playTime;
    processContext.projectTimeMusic = playTime;
    processContext.barPositionMusic = static_cast<int>(playTime / 4);
    double bpm = _track->getComposer()->_bpm;
    // TODO これあってる？
    processContext.projectTimeSamples = playTime / (bpm / 60.0) * sampleRate;
    processContext.tempo = bpm;
    processContext.timeSigNumerator = 4;
    processContext.timeSigDenominator = 4;

    processData.processContext = &processContext;

    _processor->process(processData);

    for (int bus = 0; bus < _noutputs; ++bus)
    {
        for (int channel = 0; channel < processData.outputs[bus].numChannels; ++channel)
        {
            buffer->_out[bus]._constantp[channel] =
                (processData.outputs[bus].silenceFlags & (static_cast<unsigned long long>(1) << channel)) != 0;
        }
    }

    _parameterChanges.clearQueue();

    return Module::process(buffer, steadyTime);
}

void Vst3Module::start()
{
    if (_component)
    {
        _component->setActive(true);
    }
    if (_processor)
    {
        _latency = _processor->getLatencySamples();
        if (_track && _track->getComposer())
        {
            _track->getComposer()->commandExecute(new command::ComputeLatency());
        }

        Steinberg::uint32 tailSample = _processor->getTailSamples();
        if (tailSample != Steinberg::Vst::kNoTail)
        {
            logger->debug("プラグインのテイルサンプル … {}", tailSample);
        }

        _processor->setProcessing(true);
    }
    Module::start();
}

void Vst3Module::stop()
{
    Module::stop();
    if (_processor)
    {
        _processor->setProcessing(false);
    }
    if (_component)
    {
        _component->setActive(false);
    }
}

void Vst3Module::openGui()
{
    _plugView = _controller->createView(Steinberg::Vst::ViewType::kEditor);
    if (_plugView->isPlatformTypeSupported(Steinberg::kPlatformTypeHWND) != Steinberg::kResultOk)
    {
        logger->debug("VST3 plugin does not support HWND.");
        return;
    }

    _plugView->setFrame(&_pluginContext);

    Steinberg::ViewRect size;
    _plugView->getSize(&size);

    bool resizable = _plugView->canResize() == Steinberg::kResultTrue;
    _editorWindow = std::make_unique<PluginEditorWindow>(this, size.getWidth(), size.getHeight(), resizable);
    if (_plugView->attached(_editorWindow->_hwnd, Steinberg::kPlatformTypeHWND) != Steinberg::kResultOk)
    {
        logger->debug("VST3 attached failed!");
        _editorWindow.reset();
        return;
    }

    Module::openGui();
}

void Vst3Module::closeGui()
{
    if (_plugView != nullptr)
    {
        _plugView->removed();
        _plugView->release();
        _plugView = nullptr;
    }
    if (_editorWindow != nullptr)
    {
        _editorWindow.reset();
    }
    Module::closeGui();
}

void Vst3Module::renderContent()
{
    ImGuiStyle& style = ImGui::GetStyle();
    float contentRegionMaxX = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
    float knobSize = 35.0f;
    std::vector<std::pair<Vst::ParamID, float>> startEditIds;
    std::vector<Vst::ParamID> endEditIds;
    bool isFirstLine = true;
    for (auto id : _editedParamIdList)
    {
        ImGui::PushID(id);
        auto& param = getParam(id);
        std::string title = param->getParamName();
        std::string paramString = param->getValueText();
        if (param->getStepCount() == 0)
        {
            float value = static_cast<float>(param->getValue());
            if (ImGuiKnobs::Knob(title.substr(0, 5).c_str(), &value, 0.0f, 1.0f, 0.0f, paramString.c_str(), ImGuiKnobVariant_Wiper, knobSize))
            {
                startEditIds.emplace_back(id, value);
            }
        }
        else
        {
            int value = param->getDiscreteValue();
            if (ImGuiKnobs::KnobInt(title.substr(0, 5).c_str(), &value, 0, param->getStepCount(), 1.0f, paramString.c_str(), ImGuiKnobVariant_Wiper, knobSize))
            {
                double normalizedValue = value / (double)param->getStepCount();
                startEditIds.emplace_back(id, normalizedValue);
            }
        }
        std::string tooltip = std::format("{} {} {}", _name, title, paramString);
        ImGui::SetItemTooltip(tooltip.c_str());
        if (ImGui::IsItemDeactivated())
        {
            endEditIds.push_back(id);
        }
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            AutomationTarget automationTarget(this, id);
            ImGui::SetDragDropPayload(std::format(DDP_AUTOMATION_TARGET, _track->getNekoId()).c_str(), &automationTarget, sizeof(AutomationTarget));
            ImGui::Text(tooltip.c_str());
            ImGui::EndDragDropSource();
        }
        ImGui::PopID();

        float currnetMaxX = ImGui::GetItemRectMax().x;
        float nextMaxX = currnetMaxX + style.ItemSpacing.x + knobSize;
        if (nextMaxX < contentRegionMaxX)
        {
            ImGui::SameLine();
        }
        else
        {
            if (isFirstLine)
            {
                isFirstLine = false;
            }
            else
            {
                break;
            }
        }
    }
    for (auto& [id, value] : startEditIds)
    {
        beginEdit(id);
        auto& param = getParam(id);
        param->setValue(value);
        addParameterChange(param.get(), 0, value);
    }
    for (auto id : endEditIds)
    {
        endEdit(id);
    }

    // beginEdit なしで performEdit が呼ばれたものの処理
    auto now = std::chrono::high_resolution_clock::now();
    for (auto& [id, param] : _idParamMap)
    {
        param->maybeCommit(now);
    }

    // Automation GUI Playback
    {
        std::lock_guard<std::recursive_mutex> lock(_parameterChangesMtx);
        for (auto& [id, value] : _controllerSetParamNormalizedMap)
        {
            _controller->setParamNormalized(id, value);
        }
        _controllerSetParamNormalizedMap.clear();
    }
}

void Vst3Module::onResize(int width, int height)
{
    if (_plugView)
    {
        Steinberg::ViewRect r{};
        r.right = width;
        r.bottom = height;
        Steinberg::ViewRect r2{};
        if (_plugView->getSize(&r2) == Steinberg::kResultTrue &&
            (r.getWidth() != r2.getWidth() || r.getHeight() != r2.getHeight()))
        {
            _plugView->onSize(&r);
        }
    }
}

void Vst3Module::loadState(std::filesystem::path path)
{
    auto size = std::filesystem::file_size(path);
    std::unique_ptr<char[]> buffer(new char[size]());
    std::ifstream in(path, std::ios::binary);
    if (!in)
    {
        Error("{} のステートファイルをオープンできませんでした。{}", _name, path.string());
    }
    in.read(buffer.get(), size);
    if (!in)
    {
        Error("{}のステートを読み込めませんでした。{}", _name, path.string());
    }
    uintmax_t readSize = in.gcount();
    if (readSize != size)
    {
        Error("{}のステートの読み込みサイズ不正。{}/{} {}", _name, std::to_string(readSize), std::to_string(size), path.string());
    }
    Steinberg::MemoryStream stream(buffer.get(), size);
    if (!Steinberg::Vst::PresetFile::loadPreset(&stream, _component->iid, _component, _controller))
    {
        Error("Steinberg::Vst::PresetFile::loadPreset FAILED!");
    }
    return;
}

void Vst3Module::prepareParameterInfo()
{
    _idParamMap.clear();
    for (Steinberg::int32 i = 0; i < _controller->getParameterCount(); ++i)
    {
        Steinberg::Vst::ParameterInfo parameterInfo;
        _controller->getParameterInfo(i, parameterInfo);
        Vst::ParamID id = parameterInfo.id;
        std::unique_ptr<Param> param(new Vst3Param(this, parameterInfo, _controller->getParamNormalized(id)));
        _idParamMap[id] = std::move(param);
    }
}

void Vst3Module::prepareParameterValue()
{
    for (auto& [id, param] : _idParamMap)
    {
        param->setValue(_controller->getParamNormalized(id));
        param->clearEditStatus();
    }
}

void Vst3Module::beginEdit(Steinberg::Vst::ParamID id)
{
    getParam(id)->beginEdit();
}

void Vst3Module::performEdit(Vst::ParamID id, Vst::ParamValue valueNormalized)
{
    getParam(id)->performEdit(valueNormalized);
}

void Vst3Module::endEdit(Steinberg::Vst::ParamID id)
{
    getParam(id)->endEdit();
}

void Vst3Module::setParameterValue(Vst::ParamID id, Vst::ParamValue valueNormalized)
{
    auto& param = getParam(id);
    param->setValue(valueNormalized);
    _controller->setParamNormalized(id, valueNormalized);
    // どうせ次のフレームにしか送れないので 0 でいいはず
    int32_t sampleOffset = 0;
    addParameterChange(param.get(), sampleOffset, valueNormalized);
}

void Vst3Module::addParameterChange(Param* param, int32_t sampleOffset, double value)
{
    if (_track == nullptr) return;

    std::lock_guard<std::recursive_mutex> lock(_parameterChangesMtx);

    int32 index;
    Vst::IParamValueQueue* queue = nullptr;
    for (index = 0; index < _parameterChanges.getParameterCount(); ++index)
    {
        queue = _parameterChanges.getParameterData(index);
        if (queue->getParameterId() == param->getId())
        {
            break;
        }
        queue = nullptr;
    }
    if (queue == nullptr)
    {
        queue = _parameterChanges.addParameterData(param->getId(), index);
    }
    queue->addPoint(sampleOffset, value, index);

    _controllerSetParamNormalizedMap[param->getId()] = value;
}

nlohmann::json Vst3Module::toJson(SerializeContext& context)
{
    nlohmann::json json = Module::toJson(context);
    json["type"] = "vst3";
    json["_id"] = _id;

    Steinberg::MemoryStream stream;
    if (!Steinberg::Vst::PresetFile::savePreset(&stream, _component->iid, _component, _controller))
    {
        Error("Steinberg::Vst::PresetFile::savePreset FAILED!");
    }
    json["state"] = cppcodec::base64_rfc4648::encode(stream.getData(), stream.getSize());

    return json;
}

nlohmann::json Vst3Module::scan(const std::string& path)
{
    nlohmann::json plugins = nlohmann::json::array();
    nlohmann::json plugin;

    plugin["path"] = path;

    std::string error;
    auto module = VST3::Hosting::Module::create(path, error);
    if (!module)
    {
        Error("Vst3 create error {}", error);
        return false;
    }

    std::filesystem::file_time_type writeTime = std::filesystem::last_write_time(path);
    plugin["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(writeTime.time_since_epoch()).count();

    VST3::Hosting::PluginFactory factory = module->getFactory();
    VST3::Hosting::FactoryInfo factoryInfo = factory.info();
    plugin["Factory Info"]["Vendor"] = factoryInfo.vendor();
    plugin["Factory Info"]["URL"] = factoryInfo.url();
    plugin["Factory Info"]["E-Mail"] = factoryInfo.email();
    plugin["Factory Info"]["Flags"]["Unicode"] =
        (factoryInfo.flags() & Steinberg::PFactoryInfo::FactoryFlags::kUnicode) != 0;
    plugin["Factory Info"]["Flags"]["Classes Discardable"] = factoryInfo.classesDiscardable();
    plugin["Factory Info"]["Flags"]["Component Non Discardable"] = factoryInfo.componentNonDiscardable();

    std::vector<VST3::Hosting::ClassInfo> audioClassInfo;
    for (VST3::Hosting::ClassInfo classInfo : factory.classInfos())
    {
        if (classInfo.category() != "Audio Module Class")
        {
            continue;
        }
        plugin["name"] = classInfo.name();
        plugin["id"] = classInfo.ID().toString();
        plugin["Category"] = classInfo.category();
        plugin["Vendor"] = classInfo.vendor();
        plugin["Version"] = classInfo.version();
        plugin["SDKVersion"] = classInfo.sdkVersion();
        auto& subCategories = classInfo.subCategories();
        for (auto& subCategory : subCategories)
        {
            plugin["Sub Categories"].push_back(subCategory);
        }
        plugin["Class Flags"] = classInfo.classFlags();

        plugins.push_back(plugin);
    }

    return plugins;
}

Vst3Module* Vst3Module::create(const std::string& id)
{
    std::ifstream in(configDir() / "plugin.json");
    if (!in)
    {
        return nullptr;
    }
    nlohmann::json plugins;
    in >> plugins;
    auto plugin = std::find_if(plugins["vst3"].begin(), plugins["vst3"].end(), [&id](auto x) { return x["id"] == id; });
    if (plugin == plugins["vst3"].end())
    {
        return nullptr;
    }
    auto path = (*plugin)["path"].get<std::string>();
    auto module = new Vst3Module((*plugin)["name"], nullptr);
    module->load(path);
    return module;
}

Vst3Module* Vst3Module::fromJson(const nlohmann::json& json, SerializeContext&)
{
    auto& id = json["_id"];
    auto module = Vst3Module::create(id);

    std::string state = json["state"].get<std::string>();
    std::vector<uint8_t> buffer = cppcodec::base64_rfc4648::decode(state);
    Steinberg::MemoryStream stream(buffer.data(), buffer.size());
    FUID fuid;
    fuid.fromString(json["_id"].get<std::string>().c_str());
    if (!Steinberg::Vst::PresetFile::loadPreset(&stream, module->_component->iid, module->_component, module->_controller))
    {
        Error("Steinberg::Vst::PresetFile::loadPreset FAILED!");
    }
    return module;
}

