#include "ErrorWindow.h"
#include "imgui.h"

ErrorWindow* gErrorWindow = new ErrorWindow();

void ErrorWindow::render() {
    if (!_show) {
        return;
    }
    ImGui::SetNextWindowSizeConstraints(ImVec2(300, -1), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::Begin("Error!", &_show)) {
        ImGui::TextWrapped(_message.c_str());
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
}

void ErrorWindow::show(const std::string message, const std::exception& e) {
    std::string m = message + "\n\n" + e.what();
    if (m == _message) {
        return;
    }
    _message = m;
    _show = true;
}
