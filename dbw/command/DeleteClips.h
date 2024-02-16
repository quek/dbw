#pragma once
#include <set>
#include "../Command.h"

class Clip;
class Lane;

namespace command {

class DeleteClips : public Command {
public:
    DeleteClips(std::set<std::pair<Lane*, Clip*>> clips, bool undoable);
    void execute(Composer* composer) override;
    void undo(Composer*) override;
private:
    std::vector<std::pair<Lane*, std::unique_ptr<Clip>>> _clips;
    std::set<std::pair<Lane*, Clip*>> _clipsRaw;
};
};
