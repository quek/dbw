#pragma once
#include <string>
#include "TimelineCanvasMixin.h"

class Clip;
class Composer;
class Note;

class PianoRollWindow : public TimelineCanvasMixin<Note, int16_t> {
public:
    PianoRollWindow(Composer* composer);
    virtual ~PianoRollWindow() = default;
    void render() override;
    void edit(Clip* clip);

    virtual void handleDoubleClick(Note* thing) override;
    virtual Note* handleDoubleClick(double time, int16_t* lane) override;
    virtual void handleMove(double oldTime, double newTime, int16_t* oldLane, int16_t* newLane) override;
    virtual Note* copyThing(Note* note) override;
    virtual void deleteThing(Note* note) override;

    virtual void prepareAllThings() override;

    virtual float offsetTop() const override;
    virtual float offsetLeft() const override;
    virtual float offsetStart() const override;

    virtual ImU32 colorSlectedThing() override;
    virtual ImU32 colorThing() override;

    virtual int16_t* laneFromPos(ImVec2& pos) override;
    virtual float xFromThing(Note* thing) override;
    virtual float getLaneWidth(Note* thing) override;

    bool _show = false;
    Clip* _clip = nullptr;
    std::string _scrollHereXKey = "";

protected:
    void handleShortcut() override;
    void renderPalyhead() override;
    void renderHeader() override;
    std::string windowName() override;
    std::string canvasName() override;
};
