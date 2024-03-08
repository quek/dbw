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

    Param(uint32_t id, Module* module, double value = 0.0);
    virtual ~Param() = default;
    void beginEdit();
    virtual bool canAutomate() const { return true; }
    void clearEditStatus();
    virtual void commit();
    int getDiscreteValue();
    void endEdit();
    uint32_t getId() const { return _id; }
    virtual std::string getParamName() = 0;
    virtual int32_t getStepCount() { return 0; }
    double getValue() const { return _value; }
    std::string getValueText();
    virtual std::string getValueText(double value) = 0;
    virtual void maybeCommit(std::chrono::time_point<std::chrono::high_resolution_clock> now);
    void performEdit(double value);
    void setValue(double value) { _value = value; }

protected:
    Module* _module;
    uint32_t _id;
    double _value;
    EditStatus _editStatus;
};

