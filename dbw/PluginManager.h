#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "tinyxml2/tinyxml2.h"

class Module;
class Track;

class PluginManager;
extern PluginManager gPluginManager;

class PluginManager {
public:
    PluginManager();
    Module* create(const nlohmann::json& json);
    Module* create(tinyxml2::XMLElement* element, Track* track);
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
