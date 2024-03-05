#pragma once
#include "BaseWindow.h"

class AutomationClip;

class AutomationWindow : public BaseWindow {

private:
    AutomationClip* _clip = nullptr;
};

