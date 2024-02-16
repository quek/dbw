#pragma once
#include <windows.h>
#include <pluginterfaces/vst/ivstaudioprocessor.h>
// plugproviderで必要なPluginContext用のヘッダ
#include <public.sdk/source/vst/hosting/hostclasses.h>
// 読み込んだVST3ファイルから各クラスを取得するため必要
// plugprovider.cppをプロジェクトに追加すること
#include <public.sdk/source/vst/hosting/plugprovider.h>
#include <pluginterfaces/gui/iplugview.h>
#include <json.hpp>
#include "Module.h"
#include "PluginEditorWindow.h"

class Vst3Module : public Module, public Steinberg::IPlugFrame {
public:
    Vst3Module(std::string name, Track* track);
    virtual ~Vst3Module();
    bool load(std::string path);
    virtual bool process(ProcessBuffer* buffer, int64_t steadyTime) override;
    virtual void start() override;
    virtual void stop() override;
    virtual void openGui() override;
    virtual void closeGui() override;
    void renderContent() override;
    virtual void onResize(int width, int height) override;
    virtual void loadState(std::filesystem::path path) override;
    virtual tinyxml2::XMLElement* dawProject(tinyxml2::XMLDocument* doc) override;

    static nlohmann::json scan(const std::string path);

    DECLARE_FUNKNOWN_METHODS
    Steinberg::tresult PLUGIN_API resizeView(Steinberg::IPlugView* view, Steinberg::ViewRect* newSize) SMTG_OVERRIDE;

private:
    std::string _id;
    Steinberg::Vst::HostApplication _pluginContext;
    VST3::Hosting::Module::Ptr _module = nullptr;
    std::unique_ptr<Steinberg::Vst::PlugProvider> _plugProvider = nullptr;
    Steinberg::Vst::IAudioProcessor* _processor = nullptr;
    Steinberg::Vst::IComponent* _component = nullptr;
    Steinberg::Vst::IEditController* _controller = nullptr;
    Steinberg::IPlugView* _plugView = nullptr;

    Steinberg::Vst::SymbolicSampleSizes _symbolicSampleSizes = Steinberg::Vst::SymbolicSampleSizes::kSample32;

    std::unique_ptr<PluginEditorWindow> _editorWindow = nullptr;
};
