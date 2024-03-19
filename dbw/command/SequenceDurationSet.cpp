#include "SequenceDurationSet.h"
#include "../App.h"
#include "../Composer.h"
#include "../Sequence.h"

command::SequenceDurationSet::SequenceDurationSet(Sequence* sequence, double duration) :
    _sequenceNekoRef(sequence->getNekoId()), _durationNew(duration), _durationOld(sequence->durationGet())
{
}

void command::SequenceDurationSet::execute(Composer* composer)
{
    Sequence* sequence = Neko::findByNekoId<Sequence>(_sequenceNekoRef);
    if (sequence)
    {
        std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
        sequence->durationSet(_durationNew);
    }
}

void command::SequenceDurationSet::undo(Composer* composer)
{
    Sequence* sequence = Neko::findByNekoId<Sequence>(_sequenceNekoRef);
    if (sequence)
    {
        std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
        sequence->durationSet(_durationOld);
    }
}
