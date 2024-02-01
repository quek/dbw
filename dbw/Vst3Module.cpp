#include "Vst3Module.h"
#include <public.sdk/source/vst/hosting/module.h>
#include "ErrorWindow.h"
#include "logger.h"

// ここからVSTプラグイン関係(音声処理クラスやパラメーター操作クラス)のヘッダ
#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/base/ftypes.h"
#include "pluginterfaces/vst/vsttypes.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstevents.h"

Vst3Module::Vst3Module(std::string name, Track* track) : Module(name, track) {
}

Vst3Module::~Vst3Module() {
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
    Steinberg::Vst::PluginContextFactory::instance().setPluginContext(&_pluginContext);

    std::string error;
    _module = VST3::Hosting::Module::create(path, error);
    if (!_module) {
        gErrorWindow->show(error);
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

    // PlugProviderクラスが正常に作成できたかチェック
    if (!_plugProvider) {
        // 読み込みに失敗したら終了
        logger->error("Error : PlugProviderクラスの作成に失敗");
        return false;
    }

    logger->debug("最初に見つかった音声処理クラス「{}」を作成・初期化しました。", audioClassInfo[0].name());

    // ---------------------------------------------------------------------------
    // PlugProviderクラスから音声処理クラスを取得する。(音声処理クラスは初期化済み)
    _component = _plugProvider->getComponent();
    _controller = _plugProvider->getController();

    // 音声処理クラスはIComponentクラスとIAudioProcessorクラスを継承している
    // PlugProviderクラスから取得できるのはIComponentクラスのみなので、
    // IComponentクラス取得後、queryInterfaceでIAudioProcessorクラスも取得する
    _component->queryInterface(Steinberg::Vst::IAudioProcessor::iid, (void**)&_processor);
    if (_processor == nullptr) {
        logger->error("Error : 音声処理クラスの実装が不十分なため失敗");
        return false;
    }

    // ---------------------------------------------------------------------------
    // 取得した音声処理クラスのセットアップを行う。(その1)
    // 音声入出力を確認する。

    logger->debug("==================== 音声入出力の情報 ====================");

    // 音声入力情報の取得
    Steinberg::int32 audioInNum = _component->getBusCount(Steinberg::Vst::MediaTypes::kAudio, Steinberg::Vst::BusDirections::kInput);
    logger->debug("音声入力バスは {} 個あります。", audioInNum);

    // 各音声入力バスの情報を取得する
    for (int i = 0; i < audioInNum; i++) {
        // バスの情報(名称・チャンネル数など)を取得する
        Steinberg::Vst::BusInfo busInfo;
        _component->getBusInfo(Steinberg::Vst::MediaTypes::kAudio, Steinberg::Vst::BusDirections::kInput, i, busInfo);

        // スピーカー構成のビットを取得する
        Steinberg::Vst::SpeakerArrangement arr = 0;
        _processor->getBusArrangement(Steinberg::Vst::BusDirections::kInput, i, arr);

        // logger->debug("  音声入力バス {}個目 … バス名: {}  チャンネル数(スピーカー構成フラグ)：{}ch", i + 1, busInfo.name, busInfo.channelCount/*, arr*/);
    }

    // 音声出力情報の取得(音声入力と同様の処理なので詳細なコメントは割愛)
    Steinberg::int32 audioOutNum = _component->getBusCount(Steinberg::Vst::MediaTypes::kAudio, Steinberg::Vst::BusDirections::kOutput);
    logger->debug("音声出力バスは {} 個あります。", audioOutNum);

    for (int i = 0; i < audioOutNum; i++) {
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
    Steinberg::int32 eventInNum = _component->getBusCount(Steinberg::Vst::MediaTypes::kEvent, Steinberg::Vst::BusDirections::kInput);
    logger->debug("イベント入力バスは {} 個あります。", eventInNum);

    // 各イベント入力バスの情報を取得する
    for (int i = 0; i < eventInNum; i++) {
        // バスの情報(名称・チャンネル数など)を取得する
        Steinberg::Vst::BusInfo busInfo;
        _component->getBusInfo(Steinberg::Vst::MediaTypes::kEvent, Steinberg::Vst::BusDirections::kInput, i, busInfo);

        // logger->debug("  イベント入力バス {}個目 … バス名: {}  MIDIチャンネル数：{}ch", i + 1, busInfo.name, busInfo.channelCount);
    }

    // イベント(MIDI等)出力情報の取得(イベント入力と同様の処理なので詳細なコメントは割愛)
    Steinberg::int32 eventOutNum = _component->getBusCount(Steinberg::Vst::MediaTypes::kEvent, Steinberg::Vst::BusDirections::kOutput);
    logger->debug("イベント出力バスは {} 個あります。", eventOutNum);

    for (int i = 0; i < eventInNum; i++) {
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
    Steinberg::tresult resSampleBit64 = _processor->canProcessSampleSize(Steinberg::Vst::SymbolicSampleSizes::kSample64);

    logger->debug("　32bit対応 … {}  64bit対応 … {}", (resSampleBit32 != Steinberg::kResultFalse) ? "対応" : "非対応", (resSampleBit64 != Steinberg::kResultFalse) ? "対応" : "非対応");

    Steinberg::uint32 latency = _processor->getLatencySamples();
    logger->debug("プラグインのレイテンシ     … {}", latency);

    Steinberg::uint32 tailSample = _processor->getTailSamples();
    logger->debug("プラグインのテイルサンプル … {}", tailSample);

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
    Steinberg::Vst::ProcessSetup setup{ Steinberg::Vst::ProcessModes::kRealtime, Steinberg::Vst::SymbolicSampleSizes::kSample32, blockSize, sampleRate };

    if (_processor->setupProcessing(setup) != Steinberg::kResultOk) {
        logger->debug("Error : 音声処理クラスのセットアップに失敗");
        logger->debug("      : 32bit、サンプリングレート {}Hz、ブロックサイズ {}に非対応", sampleRate, blockSize);
    } else {
        logger->debug("音声処理クラスのセットアップが完了");
        logger->debug("プラグインは32bit、サンプリングレート {}Hz、ブロックサイズ {}に対応", sampleRate, blockSize);
    }

    return true;
}

bool Vst3Module::process(ProcessBuffer* buffer, int64_t steadyTime) {
    Steinberg::Vst::Event e;
    e.type = Steinberg::Vst::Event::kNoteOnEvent;
    return false;
}

void Vst3Module::start() {
    if (_component) {
        _component->setActive(true);
    }
    if (_processor) {
        _processor->setProcessing(true);
    }
}

void Vst3Module::stop() {
    if (_processor) {
        _processor->setProcessing(false);
    }
    if (_component) {
        _component->setActive(false);
    }
}

tinyxml2::XMLElement* Vst3Module::dawProject(tinyxml2::XMLDocument* doc) {
    return nullptr;
}
