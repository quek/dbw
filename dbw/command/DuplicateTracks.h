#pragma once
#include "CopyTracksMixin.h"

namespace command {
class DuplicateTracks : public CopyTracksMixin {
public:
    DuplicateTracks(std::vector<Track*> tracks);

protected:
    void insertTracks(std::vector<std::unique_ptr<Track>>& tracks, Track* at) override;
};
};

