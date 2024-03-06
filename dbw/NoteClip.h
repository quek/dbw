#pragma once
#include "Clip.h"

class PianoWindow;

class NoteClip : public Clip {
public:
    inline static const char* TYPE = "NoteClip";
    NoteClip(double time = 0);
    NoteClip(const nlohmann::json& json);
    Clip* clone() override;
    void edit(Composer* composer) override;
    void renderInScene(PianoRollWindow* pianoRollWindow) override;
    virtual nlohmann::json toJson() override;

private:
};
