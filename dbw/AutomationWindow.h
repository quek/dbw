#pragma once
#include "TimelineMixin.h"

class AutomationClip;
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
    void handleShortcut();
    void renderHeader();

    AutomationClip* _clip = nullptr;
    Lane* _lane = nullptr;
};

