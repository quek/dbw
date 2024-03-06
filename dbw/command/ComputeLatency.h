#pragma once
#include "../Command.h"

namespace command {
class ComputeLatency : public Command {
public:
    ComputeLatency();
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;
};
};

