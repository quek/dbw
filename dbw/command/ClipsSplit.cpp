#include "ClipsSplit.h"
#include "../App.h"
#include "../Composer.h"
#include "../Clip.h"
#include "../Lane.h"

command::ClipsSplit::ClipsSplit(std::set<std::pair<Lane*, Clip*>>& targets, double time) :
    ClipsMixin(targets), _time(time)
{
}

void command::ClipsSplit::execute(Composer* composer)
{
    auto laneAndClips = laneAndClipsGet();
    _clonedNekoId.clear();

    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);

    for (auto& [lane, clip] : laneAndClips)
    {
        if (_time <= clip->timeGet() || clip->timeGet() + clip->durationGet() <= _time)
        {
            _clonedNekoId.push_back(0);
            continue;
        }

        double oldDuration = clip->durationGet();
        double newDuration = _time - clip->timeGet();
        clip->durationSet(newDuration);
        auto newClip = clip->clone();
        _clonedNekoId.push_back(newClip->getNekoId());
        newClip->timeSet(_time);
        newClip->durationSet(oldDuration - newDuration);
        lane->_clips.emplace_back(newClip);
    }
}

void command::ClipsSplit::undo(Composer* composer)
{
    auto laneAndClips = laneAndClipsGet();

    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);

    for (auto& [lane, clip] : laneAndClips)
    {
        NekoId id = _clonedNekoId.front();
        if (id == 0) continue;
        Clip* clonedClip = Neko::findByNekoId<Clip>(id);
        _clonedNekoId.erase(_clonedNekoId.begin());
        if (!clonedClip) continue;

        clip->durationSet(clip->durationGet() + clonedClip->durationGet());

        auto it = std::ranges::find_if(lane->_clips, [clonedClip](const auto& x) { return x.get() == clonedClip; });
        if (it != lane->_clips.end())
        {
            lane->_clips.erase(it);
        }
    }
}
