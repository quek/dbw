#pragma once
#include <string>
#include "TimelineCanvasMixin.h"

class Clip;
class Composer;
class Note;

class PianoRollWindow : public TimelineCanvasMixin<Note, uint16_t> {
public:
    PianoRollWindow(Composer* composer);
    virtual ~PianoRollWindow() = default;
    void render();
    void edit(Clip* clip);

    virtual float offsetTop() const override;
    virtual float offsetLeft() const override;
    virtual float offsetStart() const override;

    bool _show = false;
    Clip* _clip = nullptr;
    std::string _scrollHereYKey = "";
};
