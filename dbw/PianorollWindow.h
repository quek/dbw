#pragma once
#include <string>
#include "TimelineCanvasMixin.h"

class Clip;
class Command;
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
    virtual void handleClickTimeline(double time, bool ctrl, bool alt) override;
    virtual std::pair<std::set<Note*>, Command*> copyThings(std::set<Note*> srcs, bool redoable) override;
    virtual Command* deleteThings(std::set<Note*> notes, bool undoable) override;

    virtual void onClickThing(Note* note) override;

    virtual void prepareAllThings() override;

    virtual float offsetTop() const override;
    virtual float offsetLeft() const override;
    virtual float offsetStart() const override;

    virtual ImU32 colorSlectedThing() override;
    virtual ImU32 colorThing() override;

    virtual int16_t* laneFromPos(ImVec2& pos) override;
    virtual float xFromThing(Note* thing) override;
    virtual float laneToScreenX(int16_t* lane);
    virtual float getLaneWidth(Note* thing) override;

    bool _show = false;
    Clip* _clip = nullptr;
    std::string _scrollHereXKey = "";
    double _defaultNoteDuration = 1.0;

protected:
    void handleShortcut() override;
    void renderPlayhead() override;
    void renderHeader() override;
    std::string windowName() override;
    std::string canvasName() override;
};
