#pragma once
#include "AutomationPoint.h"
#include "Clip.h"

class AutomationClip : public Clip {
public:
    inline static const char* TYPE = "AutomationClip";
    AutomationClip(double time);
    Clip* clone() override;
    void edit(Composer* composer) override;
    std::string name() const override;
    void renderInScene(PianoRollWindow* pianoRollWindow) override;
    virtual nlohmann::json toJson() override;
};

