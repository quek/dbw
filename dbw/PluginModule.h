#pragma once
#include <clap/clap.h>
#include <memory>
#include "Module.h"

class Track;
class PluginHost;

class PluginModule : public Module {
public:
    PluginModule(std::string name, Track* track, PluginHost* pluginHost);
    ~PluginModule();
    void openGui() override;
    void closeGui() override;
    void start() override;
    void stop() override;
    bool process(ProcessBuffer* buffer, int64_t steadyTime) override;
    tinyxml2::XMLElement* dawProject(tinyxml2::XMLDocument* doc) override;

    std::unique_ptr<PluginHost> _pluginHost;
};
