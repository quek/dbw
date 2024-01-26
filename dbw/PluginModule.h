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
    void closeGui()override;
    void start() override;
    void stop() override;
    void process(ProcessBuffer* in, unsigned long framesPerBuffer, int64_t steadyTime) override;

    std::unique_ptr<PluginHost> _pluginHost;
};
