#include "AutomationClip.h"

AutomationClip::AutomationClip(double time) : Clip(time) {
}

Clip* AutomationClip::clone() {
    return new AutomationClip(*this);
}

void AutomationClip::edit(Composer*) {
    // TODO
}

std::string AutomationClip::name() const {
    return "A" + Clip::name();
}

void AutomationClip::renderInScene(PianoRollWindow*) {
    // TODO
}

nlohmann::json AutomationClip::toJson() {
    nlohmann::json json = Clip::toJson();
    json["type"] = TYPE;
    return json;
}

