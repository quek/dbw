#include "ChangeVst3ParameterValue.h"
#include "../Vst3Module.h"


command::ChangeVst3ParameterValue::ChangeVst3ParameterValue(Vst3Module* module, Vst::ParamID id, Vst::ParamValue before, Vst::ParamValue after) :
    _moduleId(module->getNekoId()), _paramId(id), _beforeValue(before), _afterValue(after) {
}

void command::ChangeVst3ParameterValue::execute(Composer*) {
    auto module = Neko::findByNekoId<Vst3Module>(_moduleId);
    if (module) {
        module->setParameterValue(_paramId, _afterValue);
    }
}

void command::ChangeVst3ParameterValue::undo(Composer*) {
    auto module = Neko::findByNekoId<Vst3Module>(_moduleId);
    if (module) {
        module->setParameterValue(_paramId, _beforeValue);
    }
}
