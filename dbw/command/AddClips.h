#pragma once
#include "../Command.h"

class Clip;
class TrackLane;

namespace command {

class AddClips : public Command {
public:
    AddClips(std::vector<std::pair<TrackLane*, Clip*>> clips, bool undoable=true);
    void execute(Composer* composer) override;
    void undo(Composer*) override;
private:
    std::vector<std::pair<TrackLane*, std::unique_ptr<Clip>>> _clips;
};
};
