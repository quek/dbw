#pragma once
#include "Clip.h"

class AudioClip : public Clip {
public:
    inline static const char* TYPE = "AudioClip";
    AudioClip(const nlohmann::json& json, SerializeContext& context);
    AudioClip(double time, const std::string& wavPath, double bpm);
    Clip* clone() override;
    void edit(Composer* composer, Lane* lane) override;
    std::string name() const override;
    void renderInScene(PianoRollWindow* pianoRollWindow) override;
    virtual nlohmann::json toJson(SerializeContext& context) override;
};

