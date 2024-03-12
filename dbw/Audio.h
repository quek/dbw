#pragma once
#include <memory>
#include "SequenceItem.h"
#include "Wav.h"


class Audio : public SequenceItem {
private:
    std::unique_ptr<Wav> _wav;
};

