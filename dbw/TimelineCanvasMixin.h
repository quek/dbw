#pragma once
#include <map>
#include <set>
#include <string>
#include <vector>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "BaseWindow.h"
#include "GridMixin.h"
#include "ZoomMixin.h"

struct Bounds;
class Composer;

template<class THING, typename LANE>
class TimelineCanvasMixin : public BaseWindow, public GridMixin, public ZoomMixin {
public:
    TimelineCanvasMixin(Composer* composer);
    virtual ~TimelineCanvasMixin() = default;

    virtual void render();

    virtual void handleDoubleClick(THING* thing) = 0;
    virtual THING* handleDoubleClick(double time, LANE* lane) = 0;
    virtual void handleMove(double oldTime, double newTime, LANE* oldLane, LANE* newLane) = 0;
    virtual void handleMouse(const ImVec2& clipRectMin, const ImVec2& clipRectMax);
    virtual void handleClickTimeline(double time) = 0;
    virtual THING* copyThing(THING*) = 0;
    virtual void deleteThing(THING*) = 0;

    std::vector<THING*> _allThings;
    std::vector<LANE*> _allLanes;
    virtual void prepareAllThings() = 0;

    void renderThings(ImVec2& windowPos);
    virtual void renderThing(THING* thing, const ImVec2& pos1, const ImVec2& pos2);
    virtual void renderTimeline();
    virtual void renderGridBeat16th(ImDrawList* drawList, float beatY, float x1, float x2);

    virtual float offsetTop() const = 0;
    virtual float offsetLeft() const = 0;
    virtual float offsetStart() const = 0;

    virtual ImU32 colorSlectedThing() = 0;
    virtual ImU32 colorThing() = 0;

    double timeFromMousePos(float offset = 0.0f, bool floor = false);

    virtual LANE* laneFromPos(ImVec2& pos) = 0;
    THING* thingAtPos(ImVec2& pos);
    virtual float xFromThing(THING* thing) = 0;
    virtual float getLaneWidth(THING* thing) = 0;
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
        std::set<THING*> _draggingThings;
        bool _unselectClickedThingIfMouserReleased = false;
        ClickedPart _thingClickedPart = Middle;
        float _thingClickedOffset = 0.0f;
        bool _rangeSelecting = false;

        std::map <THING*, Bounds> _thingBoundsMap;
        void reset() {
            _thingBoundsMap.clear();
        }
    };

    State _state;

protected:
    ImVec2 screenToCanvas(const ImVec2& pos);
    ImVec2 canvasToScreen(const ImVec2& pos);
    virtual void handleShortcut() = 0;
    virtual void renderPalyhead() = 0;
    virtual void renderHeader() = 0;
    virtual std::string windowName() = 0;
    virtual std::string canvasName() = 0;
};
