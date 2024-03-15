#include "AutomationTarget.h"
#include "Module.h"

AutomationTarget::AutomationTarget(Module* module, uint32_t paramId) :
    _module(module),
    _moduleNekoRef(module->getNekoId()),
    _paramId(paramId),
    _defaultValue(module->getParam(paramId)->getValue()) {
}

AutomationTarget::AutomationTarget(const nlohmann::json& json, SerializeContext& ) {
    _module = nullptr;
    _moduleNekoRef = json["_moduleNekoRef"];
    _paramId = json["_paramId"];
    _defaultValue = json["_defaultValue"];
}

nlohmann::json AutomationTarget::toJson(SerializeContext& ) {
    nlohmann::json json;
    json["_moduleNekoRef"] = _module->getNekoId();
    json["_paramId"] = _paramId;
    json["_defaultValue"] = _defaultValue;
    return json;
}

Module* AutomationTarget::getModule() {
    if (_module == nullptr) {
        _module = Neko::findByNekoId<Module>(_moduleNekoRef);
    }
    return _module;
}

std::string AutomationTarget::getParamName() {
    return getParam()->getParamName();
}

Param* AutomationTarget::getParam() {
    return getModule()->getParam(_paramId).get();
}
