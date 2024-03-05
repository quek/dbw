#include "AutomationClip.h"

AutomationClip::AutomationClip(double time) : Clip(time) {
}

std::string AutomationClip::name() const {
    return "A" + Clip::name();
}

