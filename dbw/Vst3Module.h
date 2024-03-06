#pragma once
#include <windows.h>
#include <pluginterfaces/vst/ivstaudioprocessor.h>
// plugproviderで必要なPluginContext用のヘッダ
#include <public.sdk/source/vst/hosting/hostclasses.h>
// 読み込んだVST3ファイルから各クラスを取得するため必要
// plugprovider.cppをプロジェクトに追加すること
#include <public.sdk/source/vst/hosting/plugprovider.h>
#include <pluginterfaces/gui/iplugview.h>
#include <nlohmann/json.hpp>
#include "Module.h"
#include "PluginEditorWindow.h"
#include "Vst3Context.h"

class Vst3Module : public Module {
public:
    inline static const char* TYPE = "vst3";
    Vst3Module(const nlohmann::json& json);
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

    void prepareParameterInfo();
    void prepareParameterValue();
    Vst::ParameterInfo* getParameterInfo(Vst::ParamID);
    int getParameterDiscreteValue(Vst::ParamID id);

    void beginEdit(Vst::ParamID);
    void performEdit(Vst::ParamID id, Vst::ParamValue valueNormalized);
    void endEdit(Vst::ParamID);
    struct ParamEditStatus {
        Vst::ParameterInfo* _paramInfo = nullptr;
        Vst::ParamValue _beforeValue = 0.0;
        bool _beginEditCalled = false;
        std::chrono::time_point<std::chrono::high_resolution_clock> _performAt;
    };
    void commitParameterValue(ParamEditStatus& status);
    Vst::ParamValue updateParameterValue(Vst::ParamID id, Vst::ParamValue valueNormalized);
    void setParameterValue(Vst::ParamID id, Vst::ParamValue valueNormalized);
    void updateEditedParamIdList(Vst::ParamID id);
    void addParameterChange(Vst::ParamID id, Vst::ParamValue valueNormalized);
    std::string getParamName(uint32_t id) override;

    virtual nlohmann::json toJson() override;
    static Vst3Module* fromJson(const nlohmann::json& json);

    static nlohmann::json scan(const std::string& path);
    static Vst3Module* create(const std::string& id);

    std::unique_ptr<PluginEditorWindow> _editorWindow = nullptr;
    Vst::IAudioProcessor* _processor = nullptr;

private:
    std::string _id;
    Vst3Context _pluginContext;
    VST3::Hosting::Module::Ptr _module = nullptr;
    std::unique_ptr<Vst::PlugProvider> _plugProvider = nullptr;
    Vst::IComponent* _component = nullptr;
    Vst::IEditController* _controller = nullptr;
    IPlugView* _plugView = nullptr;

    Vst::SymbolicSampleSizes _symbolicSampleSizes = Vst::SymbolicSampleSizes::kSample32;

    std::map<Vst::ParamID, Vst::ParameterInfo> _parameterInfoMap;
    std::map<Vst::ParamID, Vst::ParamValue> _parameterValueMap;
    std::list<Vst::ParamID> _editedParamIdList;
    std::map<Vst::ParamID, ParamEditStatus> _paramEdtiStatusMap;
    Vst::ParameterChanges _parameterChanges;
};
