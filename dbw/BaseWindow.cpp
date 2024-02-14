#include "BaseWindow.h"

bool BaseWindow::canHandleInput() {
    return ImGui::IsWindowHovered();
}

ImVec2 BaseWindow::screenToWindow(const ImVec2& pos) {
    ImVec2 windowPos = ImGui::GetWindowPos();
    return pos - windowPos;
}

ImVec2 BaseWindow::windowToScreen(const ImVec2& pos) {
    ImVec2 windowPos = ImGui::GetWindowPos();
    return pos + windowPos;
}
