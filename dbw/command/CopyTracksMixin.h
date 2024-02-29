#pragma once
#include "SelectedTracksMixin.h"

class Track;

namespace command {
class CopyTracksMixin : public SelectedTracksMixin {
public:
    CopyTracksMixin(std::vector<Track*> tracks);

protected:
    std::vector<Track*> copy(std::vector<NekoId> trackIds);

    std::vector<NekoId> _copiedTrackIds;
};
};

