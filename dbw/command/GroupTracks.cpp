#include <mutex>
#include "GroupTracks.h"
#include "../App.h"
#include "../Composer.h"
#include "../Track.h"

command::GroupTracks::GroupTracks(std::vector<Track*> tracks, bool undoable) :
    Command(undoable) {
    for (const auto& track : tracks) {
        _ids.emplace_back(track->nekoId());
    }
}

void command::GroupTracks::execute(Composer* composer) {
    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
    Track* group = new Track(std::string("Group"));
    std::vector<Track*> tracks;
    for (const auto& trackId : _ids) {
        Track* track = Neko::findByNekoId<Track>(trackId);
        if (track) {
            tracks.emplace_back(track);
            Track* parent = track->getParent();
            auto it = parent->findTrack(track);
            _undoPlaces.push_back({ parent->nekoId(), std::distance(parent->tracksBegin(), it) });
        }
    }
    if (tracks.empty()) {
        Track* parent = tracks[0]->getParent();
        auto it = parent->findTrack(tracks[0]);
        std::unique_ptr<Track> p(group);
        parent->insertTrack(it, p);

        std::vector<std::unique_ptr<Track>> deletedTracks = composer->_masterTrack->deleteTracks(tracks);
        group->insertTracks(group->tracksBegin(), deletedTracks);
    }
}

void command::GroupTracks::undo(Composer* composer) {
    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
}
