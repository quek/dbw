#include "DuplicateNotes.h"
#include "../App.h"
#include "../Composer.h"
#include "../Note.h"
#include "../Sequence.h"

command::DuplicateNotes::DuplicateNotes(Sequence* sequence, std::set<Note*>& notes, bool undoable) :
    Command(undoable), _sequenceId(sequence->getNekoId()) {
    for (auto& note : notes) {
        _noteIds.push_back(note->getNekoId());
    }
}

void command::DuplicateNotes::execute(Composer* composer) {
    Sequence* sequence = Neko::findByNekoId<Sequence>(_sequenceId);
    if (sequence == nullptr) {
        return;
    }

    std::vector<Note*> notes;
    double minTime = std::numeric_limits<double>::max();
    double maxTime = 0.0;
    for (const auto& noteId : _noteIds) {
        const Note* note = Neko::findByNekoId<Note>(noteId);
        if (note) {
            minTime = std::min(minTime, note->_time);
            maxTime = std::max(maxTime, note->_time + note->_duration);
            notes.push_back(new Note(*note));
        }
    }

    _duplicatedNoteIds.clear();
    double delta = maxTime - minTime;
    for (auto& note : notes) {
        _duplicatedNoteIds.push_back(note->getNekoId());
        note->_time += delta;
    }

    composer->_pianoRollWindow->_state._selectedThings = { notes.begin(), notes.end() };

    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
    for (auto& note : notes) {
        sequence->_items.emplace_back(note);
    }

}

void command::DuplicateNotes::undo(Composer* composer) {
    Sequence* sequence = Neko::findByNekoId<Sequence>(_sequenceId);
    if (sequence == nullptr) {
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
    for (const auto& noteId : _duplicatedNoteIds) {
        const Note* note = Neko::findByNekoId<Note>(noteId);
        if (note) {
            std::erase_if(sequence->_items, [note](auto& x) { return x.get() == note; });
        }
    }
}
