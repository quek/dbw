#include "Vst3Param.h"
#include <public.sdk/source/vst/utility/stringconvert.h>
#include "Composer.h"
#include "Track.h"
#include "Vst3Module.h"
#include "command/ChangeVst3ParameterValue.h"

Vst3Param::Vst3Param(Vst3Module* module, Vst::ParameterInfo param, double value) :
    Param(param.id, module, value), _module(module), _param(param) {
}

bool Vst3Param::canAutomate() const {
    return (_param.flags & Vst::ParameterInfo::ParameterFlags::kCanAutomate) != 0;
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

std::string Vst3Param::getValueText(double value) {
    Steinberg::Vst::String128 paramString128;
    _module->_controller->getParamStringByValue(_id, value, paramString128);
    std::string paramString = VST3::StringConvert::convert(paramString128);
    // Khz のプラグイン ナローノンブレイクスペース U+202F が入っていて化けるので
    size_t startPos = 0;
    while ((startPos = paramString.find("\xE2\x80\xAF", startPos)) != std::string::npos) {
        paramString.replace(startPos, 3, "");
        startPos += 0;
    }
    return paramString;
}

int32_t Vst3Param::getStepCount() {
    return _param.stepCount;
}
