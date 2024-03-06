#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "BaseWindow.h"
#include "GridMixin.h"
#include "ZoomMixin.h"

class Composer;

class TimelineMixin : public BaseWindow, public GridMixin, public ZoomMixin {
public:
    TimelineMixin(Composer* composer);

protected:
    virtual void handleClickTimeline(double time, bool ctrl, bool alt);
    virtual void renderTimeline();
    virtual void renderGridBeat16th(ImDrawList* drawList, float beatY, float x1, float x2);
    virtual float offsetTop() const = 0;
    virtual float offsetLeft() const = 0;
    virtual float offsetStart() const = 0;
    ImVec2 screenToCanvas(const ImVec2& pos);
    ImVec2 canvasToScreen(const ImVec2& pos);
    double timeFromMousePos(float offset = 0.0f, bool floor = false);
    virtual float timeToScreenY(double time);
    double toSnapFloor(const double time);
    double toSnapRound(const double time);

    Composer* _composer;
};

