#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

class BaseWindow {
protected:
    bool canHandleInput();
    ImVec2 screenToWindow(const ImVec2& pos);
    ImVec2 windowToScreen(const ImVec2& pos);

};

