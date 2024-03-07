#include "AutomationWindow.h"
#include "AutomationClip.h"
#include "Grid.h"
#include "Lane.h"

AutomationWindow::AutomationWindow(Composer* composer) : TimelineMixin(composer) {
    _zoomX = 1.0f;
    _zoomY = 40.0f;
    _grid = gGrids[2].get();
}

void AutomationWindow::edit(AutomationClip* clip, Lane* lane) {
    _clip = clip;
    _lane = lane;
    _show = true;
}

void AutomationWindow::render() {
    if (!_show) return;

    if (ImGui::Begin("Automation", &_show)) {
        renderGridSnap();
        renderHeader();
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        if (ImGui::BeginChild("Automation Canvas",
                              ImVec2(0.0f, 0.0f),
                              ImGuiChildFlags_None)) {
            renderTimeline();
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();

        handleShortcut();
    }
    ImGui::End();
}

void AutomationWindow::handleShortcut() {
    if (defineShortcut(ImGuiKey_Escape)) {
        _show = false;
    }
}

void AutomationWindow::renderHeader() {
    if (_lane->_automationTarget == nullptr) {
        return;
    }
    Param* param = _lane->_automationTarget->getParam();
    ImGuiIO& io = ImGui::GetIO();

}
