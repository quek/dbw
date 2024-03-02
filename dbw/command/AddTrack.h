#pragma once
#include "../Command.h"

namespace command {
class AddTrack : public Command {
public:
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;
};
};

