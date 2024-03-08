#pragma once
#include <pluginterfaces/vst/ivsteditcontroller.h>
#include "Param.h"

class Vst3Module;

using namespace Steinberg;

class Vst3Param : public Param {
public:
    Vst3Param(Vst3Module* module, Vst::ParameterInfo param, double value);
    bool canAutomate() const override;
    void commit() override;
    std::string getParamName() override;
    std::string getValueText(double value) override;
    int32_t getStepCount() override;
private:
    Vst3Module* _module;
    Vst::ParameterInfo _param;
};

