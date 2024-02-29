#pragma once
#include "CopyTracksMixin.h"

namespace command {
class CopyDragTracks : public CopyTracksMixin {
public:
    CopyDragTracks(std::vector<Track*> tracks, Track* at);

protected:
    void insertTracks(std::vector<std::unique_ptr<Track>>& tracks, Track* at) override;
};
};

