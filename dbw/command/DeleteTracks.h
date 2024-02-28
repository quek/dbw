#pragma once
#include "../Command.h"
#include "RemoveTracksMixin.h"

namespace command {
class DeleteTracks : public Command, public RemoveTracksMixin {
public:
    DeleteTracks(std::vector<Track*>& tracks);
    virtual ~DeleteTracks() = default;
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;
};
};

