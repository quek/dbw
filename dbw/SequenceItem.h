#pragma once
#include "Neko.h"

class ProcessBuffer;

class SequenceItem : public Neko {
public:
    static SequenceItem* create(const nlohmann::json& json);
    SequenceItem(const nlohmann::json& json);
    SequenceItem(double time = 0.0, double duration = 16.0);
    ~SequenceItem() = default;
    virtual void prepareProcessBuffer(ProcessBuffer* processBuffer, double begin, double end, double clipTime, double clipDuration, double oneBeatSec) = 0;
    virtual nlohmann::json toJson() override;

    double _time;
    double _duration;
};

