#pragma once
#include "../Command.h"

namespace command {
class ComputeLatency : public Command {
public:
    ComputeLatency();

    // Command を介して継承されました
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;
};
};

