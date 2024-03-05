#pragma once
#include <cstdint>
#include "SequenceItem.h"

class ProcessBuffer;

class Note :  public SequenceItem {
public:
    inline static const char* TYPE = "note";
    Note(double time = 0.0f, double duration = 1.0f, int16_t key = 64, double velocity = 0.8, int16_t channel = 0);
    virtual ~Note() = default;
    virtual nlohmann::json toJson() override;
    void prepareProcessBuffer(ProcessBuffer* processBuffer, double begin, double end, double clipTime, double clipDuration, double oneBeatSec) override;
    void prepareProcessBuffer(ProcessBuffer* processBuffer, double begin, double end, double sequenceDuration, double oneBeatSec) override;

    double _time;
    double _duration;
    int16_t _channel;
    int16_t _key;
    double _velocity;
    double _rel;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Note, _time, _duration, _channel, _key, _velocity, _rel);
};
