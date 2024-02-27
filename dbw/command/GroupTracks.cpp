#include <mutex>
#include "GroupTracks.h"
#include "../App.h"
#include "../Composer.h"
#include "../Track.h"

command::GroupTracks::GroupTracks(std::vector<Track*> tracks, bool undoable) :
    Command(undoable) {
    for (const auto& track : tracks) {
        _trackIds.emplace_back(track->nekoId());
    }
}

void command::GroupTracks::execute(Composer* composer) {
    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
    int n = std::ranges::count_if(composer->allTracks(), [](auto& track) { return track->_name.starts_with("Group"); });
    Track* group = new Track(std::format("Group{}", n + 1));
    _groupId = group->nekoId();
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
    if (!tracks.empty()) {
        Track* parent = tracks[0]->getParent();
        auto it = parent->findTrack(tracks[0]);
        std::unique_ptr<Track> p(group);
        parent->insertTrack(it, p);

        std::vector<std::unique_ptr<Track>> deletedTracks = composer->_masterTrack->deleteTracks(tracks);
        group->insertTracks(group->tracksBegin(), deletedTracks);
    }

    composer->computeProcessOrder();
}

void command::GroupTracks::undo(Composer* composer) {
    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
    std::vector<Track*> tracks;
    for (auto id : _trackIds) {
        Track* track = Neko::findByNekoId<Track>(id);
        if (!track) {
            return;
        }
        tracks.push_back(track);
    }
    std::vector<std::unique_ptr<Track>> deletedTracks = composer->_masterTrack->deleteTracks(tracks);

    for (auto [track, undoPlace] : std::views::zip(deletedTracks, _undoPlaces)) {
        Track* parent = Neko::findByNekoId<Track>(undoPlace.first);
        if (parent) {
            parent->insertTrack(parent->tracksBegin() + undoPlace.second, track);
        }
    }

    Track* group = Neko::findByNekoId<Track>(_groupId);
    if (!group) {
        return;
    }
    if (group->getTracks().empty()) {
        group->getParent()->deleteTracks({ group });
    }

    composer->computeProcessOrder();
}
