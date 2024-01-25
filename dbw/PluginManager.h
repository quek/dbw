#pragma once
#include <json.hpp>

class PluginManager
{
public:
    void scan();
    void load();
    nlohmann::json _plugins;
};

