#pragma once
#include "TimelineMixin.h"

class AutomationClip;
class AutomationPoint;
class Composer;
class Lane;

class AutomationWindow : public TimelineMixin {
public:
    AutomationWindow(Composer* composer);
    void edit(AutomationClip* clip, Lane* lane);
    void render();

    bool _show = false;

protected:
    float offsetTop() const override { return 0.0f; };
    float offsetLeft() const override { return 20.0f; };
    float offsetStart() const override { return 0.0f; };

private:
    void handleMouse();
    void handleShortcut();
    ImVec2 pointToScreenPos(const AutomationPoint& point);
    void renderHeader();
    void renderPoints();
    AutomationPoint* screenPosToPoint(ImVec2& pos);

    AutomationClip* _clip = nullptr;
    Lane* _lane = nullptr;
};

