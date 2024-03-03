#pragma once
#include <set>
#include <vector>
#include "../Command.h"

class Clip;
class Lane;

namespace command {

class DuplicateClips : public Command {
public:
    DuplicateClips(std::set<std::pair<Lane*, Clip*>>& targets, bool undoable);
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;
private:
    std::vector<std::pair<NekoId, NekoId>> _lanAndClips;
    std::vector<NekoId> _duplicatedCliIds;
};
};

