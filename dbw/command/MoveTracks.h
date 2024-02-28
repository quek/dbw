#pragma once
#include "../Command.h"
#include "RemoveTracksMixin.h"

namespace command {
class MoveTracks : public Command, public RemoveTracksMixin {
public:
    MoveTracks(std::vector<Track*>& tracks, Track* at);
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;

private:
    NekoId _atTrackId;
};
};

