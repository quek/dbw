#pragma once
#include "../Command.h"

class Tack;

namespace command {

class NewLane : public Command {
public:
    NewLane(Track* track);
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;
private:
    NekoId _trackId;
};
};

