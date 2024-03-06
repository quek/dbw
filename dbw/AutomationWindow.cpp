#include "AutomationWindow.h"
#include "AutomationClip.h"
#include "Grid.h"

AutomationWindow::AutomationWindow(Composer* composer) : TimelineMixin(composer) {
    _zoomX = 1.0f;
    _zoomY = 40.0f;
    _grid = gGrids[2].get();
}

void AutomationWindow::edit(AutomationClip* clip) {
    _clip = clip;
    _show = true;
}

void AutomationWindow::render() {
    if (!_show) return;

    if (ImGui::Begin("Automation", &_show)) {
        renderTimeline();

        handleShortcut();
    }
    ImGui::End();
}

void AutomationWindow::handleShortcut() {
    if (defineShortcut(ImGuiKey_Escape)) {
        _show = false;
    }
}
