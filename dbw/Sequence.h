#pragma once
#include <memory>
#include <vector>
#include "Command.h"
#include "Nameable.h"
#include "Note.h"

class Composer;

class Sequence : public Nameable {
public:
    Sequence(double duration = 16.0);

    std::vector<std::unique_ptr<Note>> _notes;
    double _duration;

    static int _no;
};

class DeleteNoteCommand : public Command {
public:
    DeleteNoteCommand(Sequence* sequence, Note* note, bool undoable = true);
    void execute(Composer* composer);
    void undo(Composer* composer);

    Sequence* _sequence;
    Note* _note;
    std::unique_ptr<Note> _noteForUndo;
};
