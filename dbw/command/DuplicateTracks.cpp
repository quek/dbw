#include "DuplicateTracks.h"
#include "../App.h"
#include "../Composer.h"
#include "../MasterTrack.h"
#include "../Track.h"

command::DuplicateTracks::DuplicateTracks(std::vector<Track*> tracks) :
    CopyTracksMixin(tracks) {
}

void command::DuplicateTracks::execute(Composer* composer) {
    std::vector<std::unique_ptr<Track>> tracks;
    for (auto& track : copy(_selectedTrackIds)) {
        tracks.emplace_back(track);
    }
    if (tracks.empty()) {
        return;
    }
    Track* at = Neko::findByNekoId<Track>(_selectedTrackIds.back());
    if (!at) {
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
    at->insertTracksAfterThis(tracks);
    composer->computeProcessOrder();
}

void command::DuplicateTracks::undo(Composer* composer) {
    std::vector<Track*> tracks;
    for (auto& cpiedTrackId : _copiedTrackIds) {
        Track* track = Neko::findByNekoId<Track>(cpiedTrackId);
        if (!track) {
            continue;
        }
        tracks.push_back(track);
    }
    if (tracks.empty()) {
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
    tracks[0]->getMasterTrack()->deleteTracks(tracks);
    composer->computeProcessOrder();
}

