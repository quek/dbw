#include "GroupTrack.h"
#include "../Composer.h"
#include "../Track.h"

command::GroupTrack::GroupTrack(std::vector<Track*> tracks, bool undoable) :
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

void command::GroupTrack::execute(Composer* composer) {
    Track* group = new Track(std::string("Group"));
    bool first = true;
    for (const auto& trackId : _ids) {
        Track* track = Neko::findByNekoId<Track>(trackId);
        if (!track) {
            continue;
        }
        auto it = track->getTracksHolder()->findTrack(track);
        if (it == track->getTracksHolder()->getTracks().end()) {
            continue;
        }
        _undoPlaces.push_back({ track->getTracksHolder()->nekoId(), std::distance(track->getTracksHolder()->getTracks().begin(), it) });
        group->addTrack(std::move(*it));
        if (first) {
            first = false;
            (*it).reset(group);
        } else {
            track->getTracksHolder()->getTracks().erase(it);
        }
    }
}

void command::GroupTrack::undo(Composer*) {
}
