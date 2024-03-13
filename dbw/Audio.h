#pragma once
#include <memory>
#include "SequenceItem.h"
#include "Wav.h"


class Audio : public SequenceItem {
public:
    Audio(const std::string& wavPath);
    void prepareProcessBuffer(Lane* lane, double begin, double end, double clipBegin, double clipEnd, double loopBegin, double loopEnd, double oneBeatSec) override;
private:
    std::string _wavPath;
    std::unique_ptr<Wav> _wav;
};

