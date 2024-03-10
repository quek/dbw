#include "Param.h"
#include "Module.h"

Param::Param(ParamId id, Module* module, double value) : _id(id), _module(module), _value(value) {
}

void Param::beginEdit() {
    _editStatus._isEditing = true;
    _editStatus._beginEditCalled = true;
    _editStatus._beforeValue = _value;
}

void Param::clearEditStatus() {
    _editStatus._isEditing = false;
    _editStatus._beginEditCalled = false;
}

void Param::commit() {
    clearEditStatus();
}

int Param::getDiscreteValue() {
    int discreteValue = std::min(getStepCount(), static_cast<int32_t>(_value * (getStepCount() + 1)));
    return discreteValue;
}

void Param::endEdit() {
    commit();

}

std::string Param::getValueText() {
    return getValueText(_value);
}

void Param::maybeCommit(std::chrono::time_point<std::chrono::high_resolution_clock> now) {
    if (!_editStatus._isEditing || _editStatus._beginEditCalled) {
        return;
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - _editStatus._performAt).count();
    if (elapsed < 1000) {
        return;
    }
    commit();
}

void Param::performEdit(double value) {
    if (!_editStatus._beginEditCalled) {
        _editStatus._isEditing = true;
        _editStatus._beforeValue = _value;
        _editStatus._performAt = std::chrono::high_resolution_clock::now();
    }
    _value = value;
    _module->addParameterChange(this);
    _module->updateEditedParamIdList(_id);
}
