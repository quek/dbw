#include "BuiltinModule.h"
#include <functional>
#include "GainModule.h"

std::map<std::string, std::function<BuiltinModule* ()>> builtinModuleMap = {
    {"Gain", []() -> BuiltinModule* { return new GainModule(); }},
};

BuiltinModule::BuiltinModule(std::string name, Track* track) :
    Module(name, track) {
}

BuiltinModule* BuiltinModule::create(std::string& id) {
    auto factory = builtinModuleMap.find(id);
    if (factory == builtinModuleMap.end()) {
        return nullptr;
    }
    BuiltinModule* module = (*factory).second();
    return module;
}
