#include "AutomationPoint.h"
#include "Config.h"
#include "Lane.h"
#include "Module.h"

AutomationPoint::AutomationPoint(double value, double time) {
    setValue(value);
    setTime(time);
}

AutomationPoint::AutomationPoint(const nlohmann::json& json) : SequenceItem(json) {
    _value = json["_value"];
}

void AutomationPoint::addTo(std::vector<std::unique_ptr<SequenceItem>>& items) {
    SequenceItem::addTo(items);
    std::ranges::sort(items, {}, &AutomationPoint::_time);
}

void AutomationPoint::prepareProcessBuffer(Lane* lane, double begin, double end, double clipBegin, double clipEnd, double loopBegin, double loopEnd, double oneBeatSec) {
    if (lane->_automationTarget) {
        return;
    }
    double sampleRate = gPreference.sampleRate;
    Module* module = lane->_automationTarget->getModule();
    Param* param = lane->_automationTarget->getParam();



    double pointTime = clipBegin + _time;
    if (begin < end) {
        for (int i = 0; i < gPreference.bufferSize; ++i) {

        }


        if (begin <= pointTime && pointTime < end) {
            uint32_t sampleOffsetDouble = (pointTime - begin) * oneBeatSec * sampleRate;
            uint32_t sampleOffset = std::round(sampleOffsetDouble);
            module->addParameterChange(param, sampleOffset, _value);
        }
    } else {
    }
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
