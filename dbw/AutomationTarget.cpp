#include "AutomationTarget.h"
#include "Module.h"

AutomationTarget::AutomationTarget(Module* module, uint32_t paramId) :
    _module(module), _moduleNekoRef(module->getNekoId()), _paramId(paramId) {
}

AutomationTarget::AutomationTarget(const nlohmann::json& json) {
    _module = nullptr;
    _moduleNekoRef = json["_moduleNekoRef"];
    _paramId = json["_paramId"];
}

nlohmann::json AutomationTarget::toJson() {
    nlohmann::json json;
    json["_moduleNekoRef"] = _module->getNekoId();
    json["_paramId"] = _paramId;
    return json;
}

Module* AutomationTarget::getModule() {
    if (_module == nullptr) {
        _module = Neko::findByNekoId<Module>(_moduleNekoRef);
    }
    return _module;
}

std::string AutomationTarget::getParamName() {
    return getModule()->getParamName(getParamId());
}

Param* AutomationTarget::getParam(uint32_t paramId) {
    return _module->getParam(paramId);
}
