#pragma once
#include "SequenceItem.h"

class AutomationPoint : public SequenceItem {
public:
    inline static const char* TYPE = "AutomationPoint";
    AutomationPoint(double value, double time);
    AutomationPoint(const nlohmann::json& json);
    virtual ~AutomationPoint() = default;
    void addTo( std::vector<std::unique_ptr<SequenceItem>>& items) override;
    double getValue() const { return _value; }
    void prepareProcessBuffer(ProcessBuffer* processBuffer, double begin, double end, double clipTime, double clipDuration, double oneBeatSec) override;
    void setValue(double value);
    virtual nlohmann::json toJson() override;

private:
    double _value = 0.0;
};

