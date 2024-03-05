#pragma once
#include "Neko.h"

class SequenceItem : public Neko {
public:
    static SequenceItem* create(const nlohmann::json& json);
    virtual void prepareProcessBuffer(ProcessBuffer* processBuffer, double begin, double end, double sequenceDuration, double oneBeatSec) = 0;

};

