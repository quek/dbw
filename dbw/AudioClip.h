#pragma once
#include "Clip.h"

class AudioClip : public Clip {
public:
    AudioClip(double time, const std::string& wavPath);
    Clip* clone() override;
    void edit(Composer* composer, Lane* lane) override;
    std::string name() const override;
    void renderInScene(PianoRollWindow* pianoRollWindow) override;
};

