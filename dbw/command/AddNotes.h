#pragma once
#include "../Command.h"

class Clip;
class TrackLane;

namespace command {

class AddNotes : public Command {
public:
    AddNotes(std::vector<std::pair<TrackLane*, Clip*>> notes, bool undoable);
    void execute(Composer* composer) override;
    void undo(Composer*) override;
private:
    std::filesystem::path _path;
};

};

