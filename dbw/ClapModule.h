#pragma once
#include <clap/clap.h>
#include <memory>
#include "Module.h"

class Track;
class ClapHost;

class ClapModule : public Module {
public:
    inline static const char* TYPE = "clap";
    ClapModule(const nlohmann::json& json);
    ClapModule(std::string name, Track* track, ClapHost* pluginHost);
    ~ClapModule();
    void openGui() override;
    void closeGui() override;
    void renderContent() override;
    void start() override;
    void stop() override;
    bool process(ProcessBuffer* buffer, int64_t steadyTime) override;
    tinyxml2::XMLElement* toXml(tinyxml2::XMLDocument* doc) override;

    std::unique_ptr<ClapHost> _pluginHost;
};
