#pragma once
#include <set>
#include "../Command.h"

class Note;
class Sequence;

namespace command {

class AddNotes : public Command {
public:
    AddNotes(Sequence* sequence, std::set<Note*> notes, bool undoable);
    void execute(Composer* composer) override;
    void undo(Composer*) override;
private:
    Sequence* _sequence;
    std::vector<std::unique_ptr<Note>> _notes;
    std::set<Note*> _targets;
};

};

