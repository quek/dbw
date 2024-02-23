#include "ErrorWindow.h"
#include "imgui.h"

ErrorWindow* gErrorWindow = new ErrorWindow();

void ErrorWindow::render() {
    if (!_show) {
        return;
    }
    if (_focus) {
        ImGui::SetNextWindowFocus();
        _focus = false;
    }
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowSizeConstraints(ImVec2(300, -1), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::Begin("Error!", &_show)) {
        ImGui::TextWrapped(_message.c_str());
        ImGui::Separator();
        if (ImGui::Button("Close")) {
            _show = false;
        }
        ImGui::End();
    }
}

void ErrorWindow::show(const std::string message) {
    if (message == _message) {
        return;
    }
    _message = message;
    _show = true;
    _focus = true;
}

void ErrorWindow::show(const std::string message, const std::exception& e) {
    std::string m = message + "\n\n" + e.what();
    if (m == _message) {
        return;
    }
    _message = m;
    _show = true;
}
