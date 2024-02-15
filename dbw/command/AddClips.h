#pragma once
#include <set>
#include "../Command.h"

class Clip;
class TrackLane;

namespace command {

class AddClips : public Command {
public:
    AddClips(std::set<std::pair<TrackLane*, Clip*>> clips, bool undoable);
    void execute(Composer* composer) override;
    void undo(Composer*) override;
private:
    std::vector<std::pair<TrackLane*, std::unique_ptr<Clip>>> _clips;
    std::set<std::pair<TrackLane*, Clip*>> _clipsRaw;
};
};
