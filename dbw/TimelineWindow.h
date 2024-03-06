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

    void handleDoubleClick(Clip* thing) override;
    Clip* handleDoubleClick(double time, Lane* lane) override;
    void handleMove(double oldTime, double newTime, Lane* oldLane, Lane* newLane) override;
    void handleMouse(const ImVec2& clipRectMin, const ImVec2& clipRectMax) override;
    std::pair<std::set<Clip*>, Command*> copyThings(std::set<Clip*> clips, bool redoable) override;
    Command* deleteThings(std::set<Clip*>& clips, bool undoable) override;
    Command* duplicateThings(std::set<Clip*>& things, bool undoable) override;

    void prepareAllThings() override;
    void prepareAllThings(Track* track);
    void renderThing(Clip* thing, const ImVec2& pos1, const ImVec2& pos2);

    float offsetTop() const override;
    float offsetLeft() const override;
    float offsetStart() const override;

    ImU32 colorSlectedThing() override;
    ImU32 colorThing() override;

    Lane* laneFromPos(ImVec2& pos) override;
    float xFromThing(Clip* clip) override;
    float laneToScreenX(Lane* lane);
    float getLaneWidth(Clip* clip) override;
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
