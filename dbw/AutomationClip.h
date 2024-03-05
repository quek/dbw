#pragma once
#include "AutomationPoint.h"
#include "Clip.h"

class AutomationClip : public Clip {
public:
    AutomationClip(double time);
    std::string name() const override;
};

