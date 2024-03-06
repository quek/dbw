#pragma once
#include "SequenceItem.h"

class AutomationPoint : public SequenceItem {
public:
    inline static const char* TYPE = "AutomationPoint";
    AutomationPoint(const nlohmann::json& json);
    virtual ~AutomationPoint() = default;
    void prepareProcessBuffer(ProcessBuffer* processBuffer, double begin, double end, double clipTime, double clipDuration, double oneBeatSec) override;

private:
    double _value = 0.0;

};

