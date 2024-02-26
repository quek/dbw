#pragma once
#include <map>
#include <set>
#include "TimelineCanvasMixin.h"

class Clip;
class Composer;
struct Bounds;
struct ImVec2;
class Track;
class Lane;

class TimelineWindow : public TimelineCanvasMixin<Clip, Lane> {
public:
    TimelineWindow(Composer* composer);

    virtual void handleDoubleClick(Clip* thing) override;
    virtual Clip* handleDoubleClick(double time, Lane* lane) override;
    virtual void handleMove(double oldTime, double newTime, Lane* oldLane, Lane* newLane) override;
    virtual void handleMouse(const ImVec2& clipRectMin, const ImVec2& clipRectMax) override;
    virtual void handleClickTimeline(double time, bool ctrl, bool alt) override;
    virtual std::pair<std::set<Clip*>, Command*> copyThings(std::set<Clip*> clips, bool redoable) override;
    virtual Command* deleteThings(std::set<Clip*> clips, bool undoable) override;

    virtual void prepareAllThings() override;
    void prepareAllThings(Track* track);
    virtual void renderThing(Clip* thing, const ImVec2& pos1, const ImVec2& pos2);

    virtual float offsetTop() const override;
    virtual float offsetLeft() const override;
    virtual float offsetStart() const override;

    virtual ImU32 colorSlectedThing() override;
    virtual ImU32 colorThing() override;

    virtual Lane* laneFromPos(ImVec2& pos) override;
    virtual float xFromThing(Clip* clip) override;
    virtual float laneToScreenX(Lane* lane);
    virtual float getLaneWidth(Clip* clip) override;
    float getLaneWidth(Lane* lane);

protected:
    void handleShortcut() override;
    void renderPlayhead()override;
    void renderHeader()override;
    std::string windowName() override;
    std::string canvasName() override;

private:
    float getTrackWidth(Track* track);
    float allTracksWidth();

    std::map<Lane*, float> _laneWidthMap;
    std::map<Clip*, Lane*> _clipLaneMap;
};
