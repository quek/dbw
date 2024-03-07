#include "Param.h"

Param::Param(Module* module, double value) : _module(module), _value(value) {
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

void Param::endEdit() {
    commit();

}

void Param::maybeCommit(std::chrono::time_point<std::chrono::high_resolution_clock> now) {
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - _editStatus._performAt).count();
    if (elapsed < 1000) {
        return;
    }
    commit();
}

void Param::performEdit(double value) {
    if (!_editStatus._beginEditCalled) {
        _editStatus._beforeValue = _value;
        _editStatus._performAt = std::chrono::high_resolution_clock::now();
    };
    _value = value;
    _module->addParameterChange(this);
}
