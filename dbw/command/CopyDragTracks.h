#pragma once
#include "../Command.h"
#include "CopyTracksMixin.h"

namespace command {
class CopyDragTracks : public Command, public CopyTracksMixin {
public:
    CopyDragTracks(std::vector<Track*> tracks, Track* at);
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;

private:
    NekoId _atTrackId;
};
};

