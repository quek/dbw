#pragma once
#include "../Command.h"

namespace command {
class AddTrack : public Command {
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;
};
};

