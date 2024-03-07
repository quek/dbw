#include "Vst3Param.h"
#include <public.sdk/source/vst/utility/stringconvert.h>
#include "Composer.h"
#include "Track.h"
#include "Vst3Module.h"
#include "command/ChangeVst3ParameterValue.h"

Vst3Param::Vst3Param(Vst3Module* module, Vst::ParameterInfo param, double value) :
    Param(module, value), _module(module),_param(param) {
}

void Vst3Param::commit() {
    if (_module->_track != nullptr) {
        _module->_track->getComposer()->_commandManager.executeCommand(
            new command::ChangeVst3ParameterValue(_module, _param.id, _editStatus._beforeValue, _value));
    }
    Param::commit();
}

std::string Vst3Param::getParamName() {
    return VST3::StringConvert::convert(_param.title);
}
