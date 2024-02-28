#include "RemoveTracksMixin.h"
#include "../App.h"
#include "../Composer.h"
#include "../Track.h"

command::RemoveTracksMixin::RemoveTracksMixin(std::vector<Track*>& tracks) {
    for (const auto& track : removeChildren(tracks)) {
        _trackIds.push_back(track->nekoId());
    }
}

bool command::RemoveTracksMixin::isChild(Track* track, const std::vector<Track*> tracks) {
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

std::vector<Track*> command::RemoveTracksMixin::removeChildren(const std::vector<Track*> tracks) {
    std::vector<Track*> resultTracks;
    for (const auto& track : tracks) {
        if (!isChild(track, tracks)) {
            resultTracks.push_back(track);
        }
    }
    return resultTracks;
}

std::vector<std::unique_ptr<Track>> command::RemoveTracksMixin::removeTracks() {
    std::vector<std::unique_ptr<Track>> removedTracks;
    for (const auto& trackId : _trackIds) {
        Track* track = Neko::findByNekoId<Track>(trackId);
        if (track) {
            Track* parent = track->getParent();
            auto it = parent->findTrack(track);
            _undoPlaces.push_back({ parent->nekoId(), std::distance(parent->tracksBegin(), it) });
            auto removed = track->getMasterTrack()->deleteTracks({ track });
            removedTracks.emplace_back(std::move(removed[0]));
        }
    }
    return removedTracks;
}

void command::RemoveTracksMixin::undoRemove(Composer* composer, std::vector<std::unique_ptr<Track>>& tracks) {
    for (auto [track, undoPlace] : std::views::zip(tracks, _undoPlaces) | std::views::reverse) {

        track->resolveModuleReference();

        Track* parent = Neko::findByNekoId<Track>(undoPlace.first);
        if (parent) {
            parent->insertTrack(parent->tracksBegin() + undoPlace.second, track);
        }
    }

    composer->computeProcessOrder();
}

