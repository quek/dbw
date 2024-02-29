#include "SelectedTracksMixin.h"
#include "../Track.h"

command::SelectedTracksMixin::SelectedTracksMixin(std::vector<Track*>& tracks) {
    for (const auto& track : removeChildren(tracks)) {
        _selectedTrackIds.push_back(track->getNekoId());
    }
}

bool command::SelectedTracksMixin::isChild(Track* track, const std::vector<Track*> tracks) {
    Track* parent = track->getParent();
    if (parent->isMasterTrack()) {
        return false;
    }
    for (auto& x : tracks) {
        if (parent == x) {
            return true;
        }
    }
    return isChild(parent, tracks);
}

std::vector<Track*> command::SelectedTracksMixin::removeChildren(const std::vector<Track*> tracks) {
    std::vector<Track*> resultTracks;
    for (const auto& track : tracks) {
        if (!isChild(track, tracks)) {
            resultTracks.push_back(track);
        }
    }
    return resultTracks;
}
