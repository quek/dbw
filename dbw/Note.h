#pragma once
#include <cstdint>
#include "SequenceItem.h"

class Lane;
struct ImVec2;

class Note : public SequenceItem
{
public:
    inline static const char* TYPE = "Note";
    Note(const nlohmann::json& json, SerializeContext& context);
    Note(double time = 0.0f, double duration = 1.0f, int16_t key = 64, double velocity = 0.8, int16_t channel = 0);
    virtual ~Note() = default;
    virtual nlohmann::json toJson(SerializeContext& context) override;
    void prepareProcessBuffer(Lane* lane, double begin, double end, double clipBegin, double clipEnd, double loopBegin, double loopEnd, double oneBeatSec) override;
    virtual void render(const ImVec2& screenPos1, const ImVec2& screenPos2, const bool selected);
    virtual void dragTop(double delta);

    int16_t _channel;
    int16_t _key;
    double _velocity;
    double _rel;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Note, _time, _duration, _channel, _key, _velocity, _rel);
};
