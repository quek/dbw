#include "CutTracks.h"
#include "../App.h"
#include "../Composer.h"
#include "../Track.h"

command::CutTracks::CutTracks(std::vector<Track*>& tracks) {
    for (const auto& track : tracks) {
        _trackIds.push_back(track->nekoId());
    }
}

void command::CutTracks::execute(Composer* composer) {
    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
    std::vector<Track*> tracks;
    for (const auto& trackId : _trackIds) {
        Track* track = Neko::findByNekoId<Track>(trackId);
        if (track) {
            tracks.emplace_back(track);
            Track* parent = track->getParent();
            auto it = parent->findTrack(track);
            _undoPlaces.push_back({ parent->nekoId(), std::distance(parent->tracksBegin(), it) });
        }
    }
    std::vector<std::unique_ptr<Track>> deletedTracks = composer->_masterTrack->deleteTracks(tracks);
}

void command::CutTracks::undo(Composer* composer) {
}
