#pragma once
#include <memory>
#include "SequenceItem.h"
#include "Wav.h"


class Audio : public SequenceItem {
public:
    Audio(const std::string& wavPath);
private:
    std::string _wavPath;
    std::unique_ptr<Wav> _wav;
};

