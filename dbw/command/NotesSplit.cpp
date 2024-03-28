#include "NotesSplit.h"
#include "../App.h"
#include "../Composer.h"
#include "../Note.h"
#include "../Sequence.h"

command::NotesSplit::NotesSplit(Sequence* sequence, std::set<Note*>& notes, double time) :
    _sequenceId(sequence->getNekoId()), _time(time)
{
    for (auto& note : notes)
    {
        _noteIds.push_back(note->getNekoId());
    }
}

void command::NotesSplit::execute(Composer* composer)
{
    Sequence* sequence = Neko::findByNekoId<Sequence>(_sequenceId);
    if (sequence == nullptr)
    {
        return;
    }

    _clonedNekoIds.clear();
    std::vector<Note*> notes;

    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);

    for (const auto& noteId : _noteIds)
    {
        Note* note = Neko::findByNekoId<Note>(noteId);
        if (!note)
        {
            _clonedNekoIds.push_back(0);
            continue;
        }

        if (_time <= note->timeGet() || note->timeGet() + note->durationGet() <= _time)
        {
            _clonedNekoIds.push_back(0);
            continue;
        }

        double oldDuration = note->durationGet();
        double newDuration = _time - note->timeGet();
        note->durationSet(newDuration);
        Note* clonedNote = new Note(*note);
        _clonedNekoIds.push_back(clonedNote->getNekoId());
        clonedNote->timeSet(_time);
        clonedNote->durationSet(oldDuration - newDuration);
        sequence->addItem(clonedNote);
    }
}

void command::NotesSplit::undo(Composer* composer)
{
    Sequence* sequence = Neko::findByNekoId<Sequence>(_sequenceId);
    if (sequence == nullptr)
    {
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);

    for (const auto& noteId : _noteIds)
    {
        NekoId id = _clonedNekoIds.front();
        _clonedNekoIds.erase(_clonedNekoIds.begin());
        if (id == 0) continue;

        Note* note = Neko::findByNekoId<Note>(noteId);
        if (!note) continue;
        
        Note* clonedNote = Neko::findByNekoId<Note>(id);
        if (!clonedNote) continue;

        note->durationSet(note->durationGet() + clonedNote->durationGet());

        auto it = std::ranges::find_if(sequence->getItems(), [clonedNote](const auto& x) { return x.get() == clonedNote; });
        if (it != sequence->getItems().end())
        {
            sequence->getItems().erase(it);
        }
    }
}
