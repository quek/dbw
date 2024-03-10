#include "AutomationClip.h"
#include "Composer.h"
#include "Config.h"
#include "Lane.h"
#include "Sequence.h"

AutomationClip::AutomationClip(double time) : Clip(time) {
}

AutomationClip::AutomationClip(const nlohmann::json& json) : Clip(json) {
}

Clip* AutomationClip::clone() {
    return new AutomationClip(*this);
}

void AutomationClip::edit(Composer* composer, Lane* lane) {
    composer->editAutomationClip(this, lane);
}

std::string AutomationClip::name() const {
    return "A" + Clip::name();
}

void AutomationClip::prepareProcessBuffer(Lane* lane, double begin, double end, double loopBegin, double loopEnd, double oneBeatSec) {
    auto& items = getSequence()->getItems();
    if (items.empty()) {
        return;
    }
    double sampleRate = gPreference.sampleRate;
    Module* module = lane->_automationTarget->getModule();
    Param* param = lane->_automationTarget->getParam();
    AutomationPoint* lastPoint = nullptr;
    uint32_t lastSampleOffset = 0;
    for (auto& item : items) {
        AutomationPoint* point = (AutomationPoint*)item.get();
        double pointBegin =point->getTime() + _time;
        if (begin <= pointBegin && pointBegin < end) {
            // ok
        } else {
            continue;
        }
        uint32_t sampleOffsetDouble = (point->getTime() - begin) * oneBeatSec * sampleRate;
        uint32_t sampleOffset = std::round(sampleOffsetDouble);
        double value = point->getValue();
        if (lastPoint == nullptr) {
            module->addParameterChange(param, 0, value);
        } else {
            double valueDelta = value - lastPoint->getValue();
            int sampleOffsetDelta = sampleOffset - lastSampleOffset;
            for (int i = 1; i <= sampleOffsetDelta; ++i) {
                double value = valueDelta / sampleOffsetDelta * i;
                module->addParameterChange(param, lastSampleOffset + i, value);
            }
        }

        lastPoint = point;
        lastSampleOffset = sampleOffset;
    }
}

void AutomationClip::renderInScene(PianoRollWindow*) {
    // TODO
}

nlohmann::json AutomationClip::toJson() {
    nlohmann::json json = Clip::toJson();
    json["type"] = TYPE;
    return json;
}

