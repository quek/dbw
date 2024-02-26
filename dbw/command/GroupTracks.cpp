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

std::unique_ptr<Track> findTrack(Track* track, std::vector<std::unique_ptr<Track>>& tracks, std::vector<std::ptrdiff_t>& indexes) {
    bool found = false;

    auto it = tracks.begin();;
    for (; it != tracks.end(); ++it) {
        indexes.push_back(std::distance(tracks.begin(), it));
        if ((*it).get() == track) {
            found = true;
            break;
        }
        auto result = findTrack(track, (*it)->getTracks(), indexes);
        if (result) {
            return result;
        }
        indexes.pop_back();
    }
    if (found) {
        std::unique_ptr<Track> result(std::move(*it));
        tracks.erase(it);
        return result;
    } else {
        return std::unique_ptr<Track>();
    }
}

void command::GroupTracks::execute(Composer* composer) {
    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
    Track* group = new Track(std::string("Group"));
    bool first = true;
    for (const auto& trackId : _ids) {
        Track* track = Neko::findByNekoId<Track>(trackId);
        if (!track) {
            continue;
        }
        Track* parent = track->getParent();
        auto it = parent->findTrack(track);
        if (it == parent->getTracks().end()) {
            continue;
        }
        _undoPlaces.push_back({ parent->nekoId(), std::distance(parent->getTracks().begin(), it) });
        if (first) {
            first = false;
            std::unique_ptr<Track> p(group);
            parent->insertTrack(it, p);
            it = parent->findTrack(track);
            group->addTrack(std::move(*it));
            parent->deleteTrack(it);
        } else {
            group->addTrack(std::move(*it));
            parent->deleteTrack(it);
        }
    }
}

void command::GroupTracks::undo(Composer* composer) {
    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
}
