#pragma once
#include "AutomationPoint.h"
#include "Clip.h"

class Lane;

class AutomationClip : public Clip {
public:
    inline static const char* TYPE = "AutomationClip";
    AutomationClip(double time);
    AutomationClip(const nlohmann::json& json, SerializeContext& context);
    Clip* clone() override;
    void edit(Composer* composer, Lane* lane) override;
    std::string name() const override;
    void prepareProcessBuffer(Lane* lane, double begin, double end, double loopBegin, double loopEnd, double oneBeatSec) override;
    void renderInScene(PianoRollWindow* pianoRollWindow) override;
    virtual nlohmann::json toJson(SerializeContext& context) override;
};

