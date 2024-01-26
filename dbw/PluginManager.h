#pragma once
#include <json.hpp>

class Track;

class PluginManager
{
public:
    void scan();
    void load();
    void openModuleSelector(Track* track);
    nlohmann::json _plugins;

    std::string _query;
};

