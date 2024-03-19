#pragma once
#include "../Command.h"

class Sequence;

namespace command
{
class SequenceDurationSet : public Command
{
public:
    SequenceDurationSet(Sequence* sequence, double duration);
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;
private:
    NekoId _sequenceNekoRef;
    double _durationNew;
    double _durationOld;
};
};

