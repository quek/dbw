#pragma once
#include <chrono>
#include <string>

class Module;

class Param {
public:
    struct EditStatus {
        bool _isEditing = false;
        double _beforeValue = 0.0;
        bool _beginEditCalled = false;
        std::chrono::time_point<std::chrono::high_resolution_clock> _performAt;
    };

    Param(Module* module, double value = 0.0);
    virtual ~Param() = default;
    void beginEdit();
    void clearEditStatus();
    virtual void commit();
    void endEdit();
    virtual void maybeCommit(std::chrono::time_point<std::chrono::high_resolution_clock> now);
    virtual std::string getParamName() = 0;
    double getValue() { return _value; }
    void setValue(double value) { _value = value; }
    void performEdit(double value);

protected:
    Module* _module;
    double _value;
    EditStatus _editStatus;
};

