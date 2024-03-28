#pragma once
#include "Nameable.h"

class Lane;

class SequenceItem : public Nameable
{
public:
    static SequenceItem* create(const nlohmann::json& json, SerializeContext& context);
    SequenceItem(const nlohmann::json& json, SerializeContext& context);
    SequenceItem(double time = 0.0, double duration = 16.0);
    ~SequenceItem() = default;
    virtual void addTo(std::vector<std::unique_ptr<SequenceItem>>& items);
    double durationGet() const { return _duration; }
    void durationSet(double value) { _duration = value; }
    double timeGet() const { return _time; }
    void timeSet(double time);
    virtual void prepareProcessBuffer(Lane* /*lane*/, double /*begin*/, double /*end*/, double /*clipBegin*/, double /*clipEnd*/, double /*clipOffset*/, double /*sequenceDuration*/, double /*loopBegin*/, double /*loopEnd*/, double /*oneBeatSec*/) {};
    virtual void render(const ImVec2& screenPos1, const ImVec2& screenPos2, const bool selected);
    virtual nlohmann::json toJson(SerializeContext& context) override;

    double _time;
    double _duration;
};

