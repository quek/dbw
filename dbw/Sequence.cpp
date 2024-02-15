#include "Sequence.h"

int Sequence::_no = 0;

Sequence::Sequence(double duration) : Nameable("Seq"+std::to_string(++_no)), _duration(duration) {
}

DeleteNoteCommand::DeleteNoteCommand(Sequence* sequence, Note* note, bool undoable) : Command(undoable), _sequence(sequence), _note(note) {
}

void DeleteNoteCommand::execute(Composer* /* composer */) {
    for (auto note = _sequence->_notes.begin(); note != _sequence->_notes.end(); ++note) {
        if ((*note).get() == _note) {
            _noteForUndo = std::move(*note);
            _sequence->_notes.erase(note);
            break;
        }
    }
}

void DeleteNoteCommand::undo(Composer* /*composer*/) {
    _sequence->_notes.push_back(std::move(_noteForUndo));
}
