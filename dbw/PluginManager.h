#pragma once
#include <string>
#include <json.hpp>
#include "tinyxml2/tinyxml2.h"

class Composer;
class Module;
class Track;

class PluginManager {
public:
    PluginManager(Composer* composer);
    Module* create(tinyxml2::XMLElement* element, Track* track);
    void scan();
    void load();
    void openModuleSelector(Track* track);
    nlohmann::json* findPlugin(const std::string deviceId);
    nlohmann::json _plugins;

    std::string _query;
    Composer* _composer;

private:
    void scanClap();
    void scanVst3();
};
