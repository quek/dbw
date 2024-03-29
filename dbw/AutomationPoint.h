#pragma once
#include "SequenceItem.h"

class Lane;

class AutomationPoint : public SequenceItem {
public:
    inline static const char* TYPE = "AutomationPoint";
    AutomationPoint(double value, double time);
    AutomationPoint(const nlohmann::json& json, SerializeContext& context);
    virtual ~AutomationPoint() = default;
    void addTo(std::vector<std::unique_ptr<SequenceItem>>& items) override;
    double getValue() const { return _value; }
    void setValue(double value);
    virtual nlohmann::json toJson(SerializeContext& context) override;

private:
    double _value = 0.0;
};

