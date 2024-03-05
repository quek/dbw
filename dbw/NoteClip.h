#pragma once
#include "Clip.h"

class PianoWindow;

class NoteClip : public Clip {
public:
    NoteClip(double time);
    NoteClip(const nlohmann::json& json);
    void renderInScene(PianoRollWindow* pianoRollWindow);

private:
};
