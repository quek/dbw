#include "DeleteClips.h"
#include "../Clip.h"
#include "../Lane.h"

command::DeleteClips::DeleteClips(std::set<std::pair<Lane*, Clip*>> clips, bool undoable) : Command(undoable), _clipsRaw(clips) {
}

void command::DeleteClips::execute(Composer*) {
    for (auto& raw : _clipsRaw) {
        auto it = std::ranges::find_if(raw.first->_clips, [&raw](const auto& x) { return x.get() == raw.second; });
        if (it != raw.first->_clips.end()) {
            _clips.push_back(std::pair(raw.first, std::move(*it)));
            raw.first->_clips.erase(it);
        }
    }
}

void command::DeleteClips::undo(Composer*) {
    for (auto& x : _clips) {
        x.first->_clips.push_back(std::move(x.second));
    }
    _clips.clear();
}
