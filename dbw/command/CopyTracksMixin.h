#pragma once
#include "../Command.h"
#include "SelectedTracksMixin.h"

class Track;

namespace command {
class CopyTracksMixin : public SelectedTracksMixin, public Command {
public:
    CopyTracksMixin(std::vector<Track*> tracks);
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;

protected:
    std::vector<Track*> copy(std::vector<NekoId> trackIds);
    virtual void insertTracks(std::vector<std::unique_ptr<Track>>& tracks, Track* at) = 0;

    std::vector<NekoId> _copiedTrackIds;
    NekoId _atTrackId = 0;
};
};

