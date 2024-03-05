#pragma once
#include <set>
#include <vector>
#include "../Command.h"
#include "../Neko.h"
#include "../Sequence.h"

class Note;

namespace command {
class DuplicateNotes : public Command {
public:
    DuplicateNotes(Sequence* sequence, std::set<Note*>& notes, bool undoable);
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;
private:
    NekoId _sequenceId;
    std::vector<NekoId> _noteIds;
    std::vector<NekoId> _duplicatedNoteIds;
};
};

