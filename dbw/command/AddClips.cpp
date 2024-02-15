#include "AddClips.h"
#include "../Clip.h"
#include "../TrackLane.h"

command::AddClips::AddClips(std::vector<std::pair<TrackLane*, Clip*>> clips, bool undoable) : Command(undoable) {
    for (auto& x : clips) {
        _clips.push_back(std::pair(x.first, std::unique_ptr<Clip>(x.second)));
    }
}

void command::AddClips::execute(Composer* composer) {
    for (auto& x : _clips) {
        x.first->_clips.push_back(std::move(x.second));
    }
}

void command::AddClips::undo(Composer*) {
    // TODO
}
