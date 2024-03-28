#pragma once
#include <map>
#include <set>
#include "TimelineCanvasMixin.h"
#include "TrackHeaderView.h"
#include "TrackWidthManager.h"

class Clip;
class Composer;
struct Bounds;
struct ImVec2;
class Lane;
class Track;

class TimelineWindow : public TimelineCanvasMixin<Clip, Lane>
{
public:
    TimelineWindow(Composer* composer);

    void handleDoubleClick(Clip* thing) override;
    Clip* handleDoubleClick(double time, Lane* lane) override;
    void handleMove(double oldTime, double newTime, Lane* oldLane, Lane* newLane) override;
    void handleMouse(const ImVec2& clipRectMin, const ImVec2& clipRectMax) override;
    std::pair<std::set<Clip*>, Command*> copyThings(std::set<Clip*> clips, bool redoable) override;
    Command* deleteThings(std::set<Clip*>& clips, bool undoable) override;
    Command* duplicateThings(std::set<Clip*>& things, bool undoable) override;
    Command* splitThings(std::set<Clip*>& things, double time) override;

    void prepareAllThings() override;
    void prepareAllThings(Track* track);
    void renderThing(Clip* thing, const ImVec2& pos1, const ImVec2& pos2);

    float offsetTop() const override;
    float offsetLeft() const override;
    float offsetStart() const override;

    Lane* laneFromPos(ImVec2& pos) override;
    float xFromThing(Clip* clip) override;
    float laneToScreenX(Lane* lane);
    float getLaneWidth(Clip* clip) override;

protected:
    void handleShortcut() override;
    void renderPlayhead() override;
    void renderHeader() override;
    std::string windowName() override;
    std::string canvasName() override;

private:
    std::set<std::pair<Lane*, Clip*>> commandTargets(std::set<Clip*>& clips);

    std::map<Clip*, Lane*> _clipLaneMap;
    float _headerHeight = 0.0f;
    TrackWidthManager _trackWidthManager;
    TrackHeaderView _trackHeaderView;
};
