#pragma once
#include <set>
#include <vector>
#include "../Neko.h"

class Clip;
class Lane;

namespace command
{
class ClipsMixin
{
public:
    ClipsMixin(std::set<std::pair<Lane*, Clip*>>& targets);

protected:
    std::vector<std::pair<Lane*, Clip*>> laneAndClipsGet();
    std::vector<std::pair<NekoId, NekoId>> _laneIdAndClipIds;
};
};

