#include "SelectedTracksMixin.h"
#include "../Composer.h"
#include "../Track.h"

command::SelectedTracksMixin::SelectedTracksMixin(std::vector<Track*>& tracks) {
    auto xs = removeChildren(tracks);
    std::unordered_map<Track*, size_t> indexMap;
    int index = 0;
    for (auto& track : xs[0]->getComposer()->allTracks()) {
        indexMap[track] = index++;
    }
    std::sort(xs.begin(), xs.end(), [&indexMap](Track* a, Track* b) {
        return indexMap[a] < indexMap[b];
    });

    for (const auto& track : xs) {
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
