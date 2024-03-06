#pragma once
#include "Neko.h"

class ProcessBuffer;

class SequenceItem : public Neko {
public:
    static SequenceItem* create(const nlohmann::json& json);
    virtual void prepareProcessBuffer(ProcessBuffer* processBuffer, double begin, double end, double clipTime, double clipDuration, double oneBeatSec) = 0;

    double _time;
    double _duration;
};

