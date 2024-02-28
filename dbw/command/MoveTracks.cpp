#include "MoveTracks.h"
#include "../App.h"
#include "../Composer.h"
#include "../Track.h"

command::MoveTracks::MoveTracks(std::vector<Track*>& tracks, Track* at) :
    RemoveTracksMixin(tracks), _atTrackId(at->nekoId()) {
}

void command::MoveTracks::execute(Composer* composer) {
    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);

    Track* atTrack = Neko::findByNekoId<Track>(_atTrackId);
    if (!atTrack) {
        return;
    }

    std::vector<std::unique_ptr<Track>> removedTracks = removeTracks();
    atTrack->insertTracksBeforeThis(removedTracks);

    composer->computeProcessOrder();
}

void command::MoveTracks::undo(Composer* composer) {
    std::vector<std::unique_ptr<Track>> removedTracks = removeTracks();
    undoRemove(composer, removedTracks);
}
