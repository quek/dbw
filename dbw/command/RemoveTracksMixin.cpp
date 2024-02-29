#include "RemoveTracksMixin.h"
#include "../App.h"
#include "../Composer.h"
#include "../Track.h"

command::RemoveTracksMixin::RemoveTracksMixin(std::vector<Track*>& tracks) : SelectedTracksMixin(tracks){
}


std::vector<std::unique_ptr<Track>> command::RemoveTracksMixin::removeTracks() {
    std::vector<std::unique_ptr<Track>> removedTracks;
    for (const auto& trackId : _selectedTrackIds) {
        Track* track = Neko::findByNekoId<Track>(trackId);
        if (track) {
            Track* parent = track->getParent();
            auto it = parent->findTrack(track);
            _undoPlaces.push_back({ parent->getNekoId(), std::distance(parent->tracksBegin(), it) });
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

