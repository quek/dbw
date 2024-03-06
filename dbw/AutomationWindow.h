#pragma once
#include "TimelineMixin.h"

class AutomationClip;
class Composer;

class AutomationWindow : public TimelineMixin {
public:
    AutomationWindow(Composer* composer);
    void edit(AutomationClip* clip);
    void render();

    bool _show = false;

protected:
    void handleClickTimeline(double time, bool ctrl, bool alt)override {};
    float offsetTop() const override { return 30.0f; };
    float offsetLeft() const override { return 30.0f; };
    float offsetStart() const override { return 0.0f; };

private:
    void handleShortcut();

    AutomationClip* _clip = nullptr;
    Composer* _composer;
};

