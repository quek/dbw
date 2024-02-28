#include <mutex>
#include "GroupTracks.h"
#include "../App.h"
#include "../Composer.h"
#include "../Track.h"

command::GroupTracks::GroupTracks(std::vector<Track*> tracks, bool undoable) :
    Command(undoable), RemoveTracksMixin(tracks) {
}

void command::GroupTracks::execute(Composer* composer) {
    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);

    int n = std::ranges::count_if(composer->allTracks(), [](auto& track) { return track->_name.starts_with("Group"); });
    Track* group = new Track(std::format("Group{}", n + 1));
    _groupId = group->nekoId();

    Track* firstTrack = Neko::findByNekoId<Track>(_trackIds[0]);
    if (!firstTrack) {
        return;
    }
    Track* parent = firstTrack->getParent();
    auto it = parent->findTrack(firstTrack);
    std::unique_ptr<Track> p(group);
    parent->insertTrack(it, p);

    std::vector<std::unique_ptr<Track>> removedTracks = removeTracks();
    group->insertTracks(group->tracksBegin(), removedTracks);

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

    undoRemove(composer, deletedTracks);

    Track* group = Neko::findByNekoId<Track>(_groupId);
    if (!group) {
        return;
    }
    if (group->getTracks().empty()) {
        group->getParent()->deleteTracks({ group });
    }

    composer->computeProcessOrder();
}
