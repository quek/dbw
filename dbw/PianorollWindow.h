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
    void render() override;
    void edit(Clip* clip);

    virtual void handleDoubleClick(Note* thing)override {}
    virtual void handleDoubleClick(double time, uint16_t* lane)override {}
    virtual void handleMove(double oldTime, double newTime, uint16_t* oldLane, uint16_t* newLane) override {}

    virtual void prepareAllThings()override {};

    virtual float offsetTop() const override;
    virtual float offsetLeft() const override;
    virtual float offsetStart() const override;

    virtual ImU32 colorSlectedThing() override;
    virtual ImU32 colorThing() override;

    virtual uint16_t* laneFromPos(ImVec2& pos)override { return nullptr; }
    virtual float xFromThing(Note* thing)override { return 0.0f; }
    virtual float getLaneWidth(Note* thing)override { return 0.0f; }

    bool _show = false;
    Clip* _clip = nullptr;
    std::string _scrollHereXKey = "";

protected:
    void handleShortcut() override;
    void renderPalyCursor() override;
    void renderHeader() override;
    std::string windowName() override;
    std::string canvasName() override;
};
