#pragma once
#include <pluginterfaces/vst/ivsteditcontroller.h>
#include "Param.h"

class Vst3Module;

using namespace Steinberg;

class Vst3Param : public Param {
public:
    Vst3Param(Vst3Module* module, Vst::ParameterInfo param, double value);
    void commit() override;
    std::string getParamName() override;
private:
    Vst3Module* _module;
    Vst::ParameterInfo _param;
};

