#pragma once
#include <cstdint>
#include <nlohmann/json.hpp>
#include "Neko.h"

class Module;
class Param;

class AutomationTarget {
public:
    AutomationTarget(Module* module, uint32_t paramId);
    AutomationTarget(const nlohmann::json& json, SerializeContext& context);
    virtual nlohmann::json toJson(SerializeContext& context);
    double getDefaultValue() const { return _defaultValue; }
    Module* getModule();
    uint32_t getParamId() const { return _paramId; }
    std::string getParamName();
    Param* getParam();
    void setDefaultValue(double value) { _defaultValue = value; }

private:
    Module* _module;
    NekoId _moduleNekoRef;
    uint32_t _paramId;
    double _defaultValue;
};

