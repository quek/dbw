#pragma once
#include "Clip.h"

class Lane;
class PianoWindow;

class NoteClip : public Clip {
public:
    inline static const char* TYPE = "NoteClip";
    NoteClip(double time = 0);
    NoteClip(const nlohmann::json& json, SerializeContext& context);
    Clip* clone() override;
    void edit(Composer* composer, Lane* lane) override;
    void renderInScene(PianoRollWindow* pianoRollWindow) override;
    virtual nlohmann::json toJson(SerializeContext& context) override;

private:
};
