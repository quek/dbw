#pragma once
#include <string>
#include <json.hpp>

class Track;

class PluginManager
{
public:
    void scan();
    void load();
    void openModuleSelector(Track* track);
    nlohmann::json* findPlugin(const std::string deviceId);
    nlohmann::json _plugins;

    std::string _query;
};

