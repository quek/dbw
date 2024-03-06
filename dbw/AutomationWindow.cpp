#include "AutomationWindow.h"
#include "AutomationClip.h"

AutomationWindow::AutomationWindow(Composer* composer) : _composer(composer) {
}

void AutomationWindow::edit(AutomationClip* clip) {
    _clip = clip;
    _show = true;
}

void AutomationWindow::render() {
    if (!_show) return;

    if (ImGui::Begin("Automation", &_show)) {
        ImGui::Text(_clip->name().c_str());

        handleShortcut();
    }
    ImGui::End();
}

void AutomationWindow::handleShortcut() {
    if (defineShortcut(ImGuiKey_Escape)) {
        _show = false;
    }
}
