#pragma once
#include <set>
#include <vector>
#include "../Command.h"

class Note;
class Sequence;

namespace command
{
class NotesSplit : public Command
{
public:
    NotesSplit(Sequence* sequence, std::set<Note*>& notes, double time);
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;

private:
    std::vector<NekoId> _clonedNekoIds;
    NekoId _sequenceId;
    std::vector<NekoId> _noteIds;
    std::vector<NekoId> _duplicatedNoteIds;
    double _time;
};
};

