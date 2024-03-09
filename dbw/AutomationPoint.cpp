#include "AutomationPoint.h"

AutomationPoint::AutomationPoint(double value, double time) : SequenceItem(time), _value(value) {
}

AutomationPoint::AutomationPoint(const nlohmann::json& json) : SequenceItem(json) {
    _value = json["_value"];
}

void AutomationPoint::addTo(std::vector<std::unique_ptr<SequenceItem>>& items) {
    SequenceItem::addTo(items);
    std::ranges::sort(items, {}, &AutomationPoint::_time);
}

void AutomationPoint::prepareProcessBuffer(ProcessBuffer* processBuffer, double begin, double end, double clipTime, double clipDuration, double oneBeatSec) {
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

nlohmann::json AutomationPoint::toJson() {
    nlohmann::json json = SequenceItem::toJson();
    json["type"] = TYPE;
    json["_value"] = _value;
    return json;
}
