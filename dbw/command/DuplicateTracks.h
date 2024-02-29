#pragma once
#include "../Command.h"
#include "CopyTracksMixin.h"

namespace command {
class DuplicateTracks  : public Command, public CopyTracksMixin {
public:
    DuplicateTracks(std::vector<Track*> tracks);
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;
};
};

