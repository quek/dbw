#include "NewLane.h"
#include "../App.h"
#include "../Composer.h"
#include "../Lane.h"
#include "../Track.h"

command::NewLane::NewLane(Track* track) : _trackId(track->getNekoId()){
}

void command::NewLane::execute(Composer* composer) {
    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
    Track* track = Neko::findByNekoId<Track>(_trackId);
    if (track == nullptr) {
        return;
    }
    track->addLane(new Lane());
}

void command::NewLane::undo(Composer* composer) {
    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
    Track* track = Neko::findByNekoId<Track>(_trackId);
    if (track == nullptr) {
        return;
    }
    track->_lanes.pop_back();
}
