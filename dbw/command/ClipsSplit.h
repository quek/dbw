#pragma once
#include <set>
#include "ClipsMixin.h"
#include "../Command.h"

class Clip;
class Lane;

namespace command {
class ClipsSplit : public Command, public ClipsMixin
{
public:
    ClipsSplit(std::set<std::pair<Lane*, Clip*>>& targets, double time);
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;

private:
    std::vector<NekoId> _clonedNekoId;
    double _time;
};
};

