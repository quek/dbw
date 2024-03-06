#pragma once
#include "BaseWindow.h"

class AutomationClip;
class Composer;

class AutomationWindow : public BaseWindow {
public:
    AutomationWindow(Composer* composer);
    void edit(AutomationClip* clip);
    void render();

    bool _show = false;
private:
    void handleShortcut();

    AutomationClip* _clip = nullptr;
    Composer* _composer;
};

