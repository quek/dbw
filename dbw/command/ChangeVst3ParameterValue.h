#pragma once
#include <pluginterfaces/vst/vsttypes.h>
#include "../Command.h"

using namespace Steinberg;

class Vst3Module;

namespace command {
class ChangeVst3ParameterValue :public Command {
public:
    ChangeVst3ParameterValue(Vst3Module* module, Vst::ParamID id, Vst::ParamValue before, Vst::ParamValue after);
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;

private:
    NekoId _moduleId;
    Vst::ParamID _paramId;
    Vst::ParamValue _beforeValue;
    Vst::ParamValue _afterValue;
};
};

