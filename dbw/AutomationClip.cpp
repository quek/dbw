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
    int currentFramesPerBuffer = lane->_track->getComposer()->_currentFramesPerBuffer;
    double sampleRate = gPreference.sampleRate;
    Module* module = lane->_automationTarget->getModule();
    Param* param = lane->_automationTarget->getParam();
    AutomationPoint* lastPoint = nullptr;
    int lastSampleOffset = -1;
    for (auto& item : items) {
        AutomationPoint* point = (AutomationPoint*)item.get();
        double pointBegin = point->getTime() + _time;
        double sampleOffsetDouble = (pointBegin - begin) * oneBeatSec * sampleRate;
        int sampleOffset = std::round(sampleOffsetDouble);
        double value = point->getValue();
        if (begin <= pointBegin) {
            if (lastPoint == nullptr) {
                module->addParameterChange(param, 0, value);
            } else {
                double valueDelta = value - lastPoint->getValue();
                int sampleOffsetDelta = sampleOffset - lastSampleOffset;

                int startFrame = std::max(0, lastSampleOffset + 1);
                int endFrame = std::min(startFrame + currentFramesPerBuffer, sampleOffset + 1);
                for (int frame = startFrame; frame < endFrame; ++frame) {
                    double valueAtFrame = valueDelta / sampleOffsetDelta * (frame - lastSampleOffset) + lastPoint->getValue();
                    module->addParameterChange(param, frame, valueAtFrame);
                }
            }
            if (end < pointBegin) {
                break;
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

