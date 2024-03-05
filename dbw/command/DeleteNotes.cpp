#include "DeleteNotes.h"
#include "../Note.h"
#include "../Sequence.h"

namespace command {
DeleteNotes::DeleteNotes(Sequence* sequence, std::set<Note*>& notes, bool undoable) :
    Command(undoable), _sequence(sequence), _targets(notes) {
}
void DeleteNotes::execute(Composer*) {
    for (auto& note : _targets) {
        auto it = std::ranges::find_if(_sequence->_notes, [&note](const auto& x) { return x.get() == note; });
        if (it != _sequence->_notes.end()) {
            _notes.emplace_back(std::move(*it));
            _sequence->_notes.erase(it);
        }
    }

}
void DeleteNotes::undo(Composer*) {
    for (auto& note : _notes) {
        _sequence->_notes.emplace_back(std::move(note));
    }
    _notes.clear();
}
}
