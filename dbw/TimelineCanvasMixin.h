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
class Command;
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
    virtual void handleClickTimeline(double time, bool ctrl, bool alt) = 0;
    virtual std::pair<std::set<THING*>, Command*> copyThings(std::set<THING*> srscs, bool redoable) = 0;
    virtual Command* deleteThings(std::set<THING*>& things, bool undoable) = 0;
    virtual Command* duplicateThings(std::set<THING*>& things, bool undoable) = 0;

    virtual void onClickThing(THING*) {};

    std::vector<THING*> _allThings;
    std::vector<LANE*> _allLanes;
    virtual void prepareAllThings() = 0;

    void renderThings();
    virtual void renderThing(THING* thing, const ImVec2& pos1, const ImVec2& pos2);
    virtual void renderTimeline();
    virtual void renderGridBeat16th(ImDrawList* drawList, float beatY, float x1, float x2);
    virtual void renderEditCursor();

    virtual float offsetTop() const = 0;
    virtual float offsetLeft() const = 0;
    virtual float offsetStart() const = 0;

    virtual ImU32 colorSlectedThing() = 0;
    virtual ImU32 colorThing() = 0;

    double timeFromMousePos(float offset = 0.0f, bool floor = false);

    virtual LANE* laneFromPos(ImVec2& pos) = 0;
    THING* thingAtPos(ImVec2& pos);
    virtual float xFromThing(THING* thing) = 0;
    virtual float laneToScreenX(LANE*) = 0;
    virtual float timeToScreenY(double time);
    virtual float getLaneWidth(THING* thing) = 0;
    double toSnapFloor(const double time);
    double toSnapRound(const double time);

    Composer* _composer;
    bool _show = false;

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
        ImVec2 _editCursorPos{ 0.0f, 0.0f };
        double _defaultThingDuration = 1.0;

        std::map <THING*, Bounds> _thingBoundsMap;
        void reset() {
            _thingBoundsMap.clear();
        }
    };

    State _state;

protected:
    ImVec2 screenToCanvas(const ImVec2& pos);
    ImVec2 canvasToScreen(const ImVec2& pos);
    virtual void handleShortcut();
    virtual void renderPlayhead() = 0;
    virtual void renderHeader() = 0;
    virtual std::string windowName() = 0;
    virtual std::string canvasName() = 0;
};
