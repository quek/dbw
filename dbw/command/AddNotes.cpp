#include "AddNotes.h"
#include "../Sequence.h"

command::AddNotes::AddNotes(Sequence* sequence, std::set<Note*> notes, bool undoable) :
    Command(undoable), _sequence(sequence), _targets(notes) {
    for (auto& x : _targets) {
        _notes.emplace_back(x);
    }
}

command::AddNotes::~AddNotes() {
}

void command::AddNotes::execute(Composer*) {
    for (auto& note : _notes) {
        _sequence->_items.emplace_back(std::move(note));
    }
    _notes.clear();
}

void command::AddNotes::undo(Composer*) {
    for (auto& note : _targets) {
        auto it = std::ranges::find_if(_sequence->_items, [&note](const auto& x) { return x.get() == note; });
        if (it != _sequence->_items.end()) {
            Note* p = dynamic_cast<Note*>(it->release());
            _notes.emplace_back(p);
            _sequence->_items.erase(it);
        }
    }
}
