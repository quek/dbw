#pragma once
#include "SequenceItem.h"

class AutomationPoint : public SequenceItem {
public:
    inline static const char* TYPE = "AutomationPoint";
    AutomationPoint(double value, double time);
    AutomationPoint(const nlohmann::json& json);
    virtual ~AutomationPoint() = default;
    void prepareProcessBuffer(ProcessBuffer* processBuffer, double begin, double end, double clipTime, double clipDuration, double oneBeatSec) override;

    double _value = 0.0;
};

