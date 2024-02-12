#pragma once
#include <map>
#include <set>
#include "TimelineCanvasMixin.h"

class Clip;
class Composer;
struct Bounds;
struct ImVec2;
class Track;
class TrackLane;

class TimelineWindow : public TimelineCanvasMixin<Clip, TrackLane> {
public:
    TimelineWindow(Composer* composer);

    virtual void handleDoubleClick(Clip* thing) override;
    virtual void handleDoubleClick(double time, TrackLane* lane) override;
    virtual void handleMove(double oldTime, double newTime, TrackLane* oldLane, TrackLane* newLane) override;

    virtual void prepareAllThings() override;

    virtual float offsetTop() const override;
    virtual float offsetLeft() const override;
    virtual float offsetStart() const override;

    virtual ImU32 colorSlectedThing() override;
    virtual ImU32 colorThing() override;

    virtual TrackLane* laneFromPos(ImVec2& pos) override;
    virtual float xFromThing(Clip* clip) override;
    virtual float getLaneWidth(Clip* clip) override;
    float getLaneWidth(TrackLane* lane);

protected:
    void handleShortcut() override;
    void renderPalyCursor()override;
    void renderHeader()override;
    std::string windowName() override;
    std::string canvasName() override;

private:
    float getTrackWidth(Track* track);
    float allTracksWidth();

    std::map<TrackLane*, float> _laneWidthMap;
    std::map<Clip*, TrackLane*> _clipLaneMap;
};
