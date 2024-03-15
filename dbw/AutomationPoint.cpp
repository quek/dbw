#include "AutomationPoint.h"
#include "Config.h"
#include "Lane.h"
#include "Module.h"

AutomationPoint::AutomationPoint(double value, double time) {
    setValue(value);
    setTime(time);
}

AutomationPoint::AutomationPoint(const nlohmann::json& json, SerializeContext& context) : SequenceItem(json, context) {
    _value = json["_value"];
}

void AutomationPoint::addTo(std::vector<std::unique_ptr<SequenceItem>>& items) {
    SequenceItem::addTo(items);
    std::ranges::sort(items, {}, &AutomationPoint::_time);
}

void AutomationPoint::setValue(double value) {
    if (value < 0.0) {
        _value = 0.0;
    } else if (value > 1.0) {
        _value = 1.0;
    } else {
        _value = value;
    }
}

nlohmann::json AutomationPoint::toJson(SerializeContext& context) {
    nlohmann::json json = SequenceItem::toJson(context);
    json["type"] = TYPE;
    json["_value"] = _value;
    return json;
}
