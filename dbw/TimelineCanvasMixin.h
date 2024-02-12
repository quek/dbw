#pragma once
#include <map>
#include <set>
#include <vector>
#include <imgui.h>
#include "GridMixin.h"
#include "ZoomMixin.h"

struct Bounds;
class Composer;

template<class THING, typename LANE>
class TimelineCanvasMixin : public GridMixin, public ZoomMixin {
public:
    TimelineCanvasMixin(Composer* composer);
    virtual ~TimelineCanvasMixin() = default;

    virtual void handleDoubleClick(THING* thing) = 0;
    virtual void handleDoubleClick(double time, LANE* lane) = 0;
    virtual void handleMove(double oldTime, double newTime, LANE* oldLane, LANE* newLane) = 0;
    void handleMouse(ImVec2& clipRectMin, ImVec2& clipRectMax);

    std::vector<THING*> _allThings;
    std::vector<LANE*> _allLanes;
    virtual void prepareAllThings() = 0;

    void renderThing(ImVec2& windowPos);
    virtual void renderTimeline();
    virtual void renderGridBeat16th(ImDrawList* drawList, float beatY, float x1, float x2);

    virtual float offsetTop() const = 0;
    virtual float offsetLeft() const = 0;
    virtual float offsetStart() const = 0;

    virtual ImU32 colorSlectedThing() = 0;
    virtual ImU32 colorThing() = 0;

    double timeFromMousePos(float offset = 0.0f);

    virtual LANE* laneFromPos(ImVec2& pos) = 0;
    THING* thingAtPos(ImVec2& pos);
    virtual float xFromThing(THING* thing) = 0;
    virtual float getLaneWidth(THING* lane) = 0;
    ImVec2 toCanvasPos(ImVec2& pos) const;
    double toSnapFloor(const double time);
    double toSnapRound(const double time);

    Composer* _composer;

    enum ClickedPart {
        Top,
        Middle,
        Bottom
    };

    struct State {
        THING* _clickedThing = nullptr;
        std::set<THING*> _selectedThings;
        std::set<THING*> _selectedThingsAtStartRangeSelecting;
        THING* _draggingThing = nullptr;
        bool _unselectClickedThingIfMouserReleased = false;
        ClickedPart _thingClickedPart = Middle;
        float _thingClickedOffset = 0.0f;
        bool _rangeSelecting = false;

        bool _consumedDoubleClick = false;
        bool _consumedClicked = false;
        std::map <THING*, Bounds> _thingBoundsMap;
        void reset() {
            _consumedDoubleClick = false;
            _consumedClicked = false;
            _thingBoundsMap.clear();
        }
    };

    State _state;
};
