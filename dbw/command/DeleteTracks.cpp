#include "DeleteTracks.h"
#include "../App.h"
#include "../Composer.h"
#include "../Track.h"

command::DeleteTracks::DeleteTracks(std::vector<Track*>& tracks) {
    // TODO parent が tracks に含まれているものは除外する？
    for (const auto& track : tracks) {
        _trackIds.push_back(track->nekoId());
    }
}

void command::DeleteTracks::execute(Composer* composer) {
    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);

    nlohmann::json array = nlohmann::json::array();
    for (const auto& trackId : _trackIds) {
        Track* track = Neko::findByNekoId<Track>(trackId);
        if (track) {
            array.push_back(track->toJson());
            Track* parent = track->getParent();
            auto it = parent->findTrack(track);
            _undoPlaces.push_back({ parent->nekoId(), std::distance(parent->tracksBegin(), it) });
            composer->_masterTrack->deleteTracks({ track });
        }
    }
    _jsonTracks["tracks"] = array;

    composer->computeProcessOrder();
}

void command::DeleteTracks::undo(Composer* composer) {
    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);

    std::vector<std::unique_ptr<Track>> tracks;
    for (auto& jsonTrack : _jsonTracks["tracks"]) {
        tracks.emplace_back(new Track(jsonTrack));
    }

    for (auto [track, undoPlace] : std::views::zip(tracks, _undoPlaces) | std::views::reverse) {
        Track* parent = Neko::findByNekoId<Track>(undoPlace.first);
        if (parent) {
            parent->insertTrack(parent->tracksBegin() + undoPlace.second, track);
        }
    }

    composer->computeProcessOrder();
}
