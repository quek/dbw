#pragma once
#include "SelectedTracksMixin.h"
#include <vector>
#include "../Neko.h"

class Track;

namespace command {

class SelectedTracksMixin {
public:
    SelectedTracksMixin(std::vector<Track*>& tracks);
    virtual ~SelectedTracksMixin() = default;

protected:
    bool isChild(Track* track, const std::vector<Track*> tracks);
    std::vector<Track*> removeChildren(const std::vector<Track*> tracks);

    std::vector<NekoId> _selectedTrackIds;
};

};
