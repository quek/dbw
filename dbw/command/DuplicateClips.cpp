#include "DuplicateClips.h"
#include "../App.h"
#include "../Composer.h"
#include "../Clip.h"
#include "../Lane.h"

command::DuplicateClips::DuplicateClips(std::set<std::pair<Lane*, Clip*>>& targets, bool undoable) :
    Command(undoable), ClipsMixin(targets) {
}

void command::DuplicateClips::execute(Composer* composer) {
    _landIdAndduplicatedClipId.clear();
    std::vector<std::pair<Lane*, Clip*>> laneAndClips;
    std::vector<Clip*> duplicatedClips;
    for (auto& [laneId, clipId] : _laneIdAndClipIds) {
        Lane* lane = Neko::findByNekoId<Lane>(laneId);
        if (lane == nullptr) {
            continue;
        }
        Clip* clip = Neko::findByNekoId<Clip>(clipId);
        if (clip == nullptr) {
            continue;
        }
        Clip* duplicatedClip = clip->clone();
        _landIdAndduplicatedClipId.emplace_back(laneId, duplicatedClip->getNekoId());
        laneAndClips.emplace_back(lane, duplicatedClip);
        duplicatedClips.push_back(duplicatedClip);
    }
    if (laneAndClips.empty()) {
        return;
    }

    double minTime = std::numeric_limits<double>::max();
    double maxTime = 0.0;
    for (auto& [_, clip] : laneAndClips) {
        minTime = std::min(minTime, clip->_time);
        maxTime = std::max(maxTime, clip->_time + clip->_duration);
    }
    double delta = maxTime - minTime;

    for (auto& [_, clip] : laneAndClips) {
        clip->_time += delta;
    }

    composer->_timelineWindow->_state._selectedThings = { duplicatedClips.begin(), duplicatedClips.end() };

    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
    for (auto& [lane, clip] : laneAndClips) {
        lane->_clips.emplace_back(clip);
    }
}

void command::DuplicateClips::undo(Composer* composer) {
    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
    for (const auto& [laneId, clipId] : _landIdAndduplicatedClipId) {
        Lane* lane = Neko::findByNekoId<Lane>(laneId);
        const Clip* clip = Neko::findByNekoId<Clip>(clipId);
        if (lane && clip) {
            std::erase_if(lane->_clips, [clip](auto& x) { return x.get() == clip; });
        }
    }
}
