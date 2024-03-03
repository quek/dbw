#pragma once
#include <cstdint>
#include <nlohmann/json.hpp>
#include "Neko.h"

class Module;

class AutomationTarget {
public:
    AutomationTarget(Module* module, uint32_t paramId);
    AutomationTarget(const nlohmann::json& json);
    virtual nlohmann::json toJson();
    Module* getModule();
    uint32_t getParamId() { return _paramId; }
    std::string getParamName();

private:
    Module* _module;
    NekoId _moduleNekoRef;
    uint32_t _paramId;
};

