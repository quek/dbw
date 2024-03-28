#include "ClipsMixin.h"
#include "../Clip.h"
#include "../Lane.h"

command::ClipsMixin::ClipsMixin(std::set<std::pair<Lane*, Clip*>>& targets)
{
    for (auto& [lane, clip] : targets) {
        _laneIdAndClipIds.emplace_back(lane->getNekoId(), clip->getNekoId());
    }
}

std::vector<std::pair<Lane*, Clip*>> command::ClipsMixin::laneAndClipsGet()
{
    std::vector<std::pair<Lane*, Clip*>> laneAndClips;
    for (auto& [laneId, clipId] : _laneIdAndClipIds) {
        Lane* lane = Neko::findByNekoId<Lane>(laneId);
        if (lane == nullptr) {
            continue;
        }
        Clip* clip = Neko::findByNekoId<Clip>(clipId);
        if (clip == nullptr) {
            continue;
        }
        laneAndClips.emplace_back(lane, clip);
    }
    return laneAndClips;
}
