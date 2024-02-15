#include "AddClips.h"
#include "../Clip.h"
#include "../TrackLane.h"

command::AddClips::AddClips(std::set<std::pair<TrackLane*, Clip*>> clips, bool undoable) : Command(undoable), _clipsRaw(clips) {
    for (auto& x : _clipsRaw) {
        _clips.push_back(std::pair(x.first, std::unique_ptr<Clip>(x.second)));
    }
}

void command::AddClips::execute(Composer*) {
    for (auto& x : _clips) {
        x.first->_clips.push_back(std::move(x.second));
    }
    _clips.clear();
}

void command::AddClips::undo(Composer*) {
    for (auto& raw : _clipsRaw) {
        auto it = std::ranges::find_if(raw.first->_clips, [&raw](const auto& x) { return x.get() == raw.second; });
        if (it != raw.first->_clips.end()) {
            _clips.push_back(std::pair(raw.first, std::move(*it)));
            raw.first->_clips.erase(it);
        }
    }
}
