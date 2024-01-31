#pragma once
#include "Module.h"
#include <windows.h>
#include "pluginterfaces/vst/ivstaudioprocessor.h"
// 読み込んだVST3ファイルから各クラスを取得するため必要
// plugprovider.cppをプロジェクトに追加すること
#include "public.sdk/source/vst/hosting/plugprovider.h"

class Vst3Module : public Module {
public:
    Vst3Module(std::string name, Track* track);
    virtual ~Vst3Module();
    bool load(std::string path);
    virtual void start() override;
    virtual void stop() override;
    virtual tinyxml2::XMLElement* dawProject(tinyxml2::XMLDocument* doc) override;

private:
    VST3::Hosting::Module::Ptr _module = nullptr;
    std::unique_ptr<Steinberg::Vst::PlugProvider> _plugProvider = nullptr;
    Steinberg::Vst::IAudioProcessor* _processor = nullptr;
    Steinberg::Vst::IComponent* _component = nullptr;
};
