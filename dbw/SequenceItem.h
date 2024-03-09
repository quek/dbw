#pragma once
#include "Neko.h"

class ProcessBuffer;

class SequenceItem : public Neko {
public:
    static SequenceItem* create(const nlohmann::json& json);
    SequenceItem(const nlohmann::json& json);
    SequenceItem(double time = 0.0, double duration = 16.0);
    ~SequenceItem() = default;
    virtual void addTo(std::vector<std::unique_ptr<SequenceItem>>& items);
    double getDuration() const { return _duration; }
    double getTime() const { return _time; }
    virtual void prepareProcessBuffer(ProcessBuffer* processBuffer, double begin, double end, double clipTime, double clipDuration, double oneBeatSec) = 0;
    void setTime(double time);
    virtual nlohmann::json toJson() override;

    double _time;
    double _duration;
};

