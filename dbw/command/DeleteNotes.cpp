#include "DeleteNotes.h"
#include "../Note.h"
#include "../Sequence.h"

namespace command {
DeleteNotes::DeleteNotes(Sequence* sequence, std::set<Note*>& notes, bool undoable) :
    Command(undoable), _sequence(sequence), _targets(notes) {
}
void DeleteNotes::execute(Composer*) {
    for (auto& note : _targets) {
        auto it = std::ranges::find_if(_sequence->getItems(), [&note](const auto& x) { return x.get() == note; });
        if (it != _sequence->getItems().end()) {
            Note* p = dynamic_cast<Note*>((*it).release());
            _notes.emplace_back(p);
            _sequence->getItems().erase(it);
        }
    }

}
void DeleteNotes::undo(Composer*) {
    for (auto& note : _notes) {
        _sequence->getItems().emplace_back(std::move(note));
    }
    _notes.clear();
}
}
