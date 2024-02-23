#pragma once
#include <string>
#include <nlohmann/json.hpp>

class Module;
class Track;

class PluginManager;
extern PluginManager gPluginManager;

class PluginManager {
public:
    PluginManager();
    Module* create(const nlohmann::json& json);
    void scan();
    void load();
    void openModuleSelector(Track* track);
    nlohmann::json* findPlugin(const char* pluginType, const std::string deviceId);
    nlohmann::json _plugins;

    std::string _query;

private:
    void scanClap();
    void scanVst3();
};
