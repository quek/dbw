#include "BuiltinModule.h"
#include <functional>
#include "GainModule.h"

std::map<std::string, std::function<BuiltinModule* ()>> builtinModuleMap = {
    {"Gain", []() -> BuiltinModule* { return new GainModule(); }},
};

BuiltinModule::BuiltinModule(const nlohmann::json& json, SerializeContext& context) : Module(json, context) {
}

BuiltinModule::BuiltinModule(std::string name, Track* track) :
    Module(name, track) {
}

nlohmann::json BuiltinModule::toJson(SerializeContext& context) {
    nlohmann::json json = Module::toJson(context);
    json["type"] = TYPE;
    return json;
}

BuiltinModule* BuiltinModule::create(std::string& id) {
    auto factory = builtinModuleMap.find(id);
    if (factory == builtinModuleMap.end()) {
        return nullptr;
    }
    BuiltinModule* module = (*factory).second();
    return module;
}
