#include "AutomationClip.h"
#include "Composer.h"

AutomationClip::AutomationClip(double time) : Clip(time) {
}

AutomationClip::AutomationClip(const nlohmann::json& json) : Clip(json) {
}

Clip* AutomationClip::clone() {
    return new AutomationClip(*this);
}

void AutomationClip::edit(Composer* composer) {
    composer->editAutomationClip(this);
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

