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
            renderPoints();
            handleMouse();
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();

        handleShortcut();
    }
    ImGui::End();
}

void AutomationWindow::handleMouse() {
    if (!canHandleInput()) {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mousePos = io.MousePos;
    auto point = screenPosToPoint(mousePos);

    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        _clip->_sequence->_items.emplace_back(point);
    }
}

void AutomationWindow::handleShortcut() {
    if (defineShortcut(ImGuiKey_Escape)) {
        _show = false;
    }
}

ImVec2 AutomationWindow::pointToScreenPos(AutomationPoint* point) {
    float x = point->_value * ImGui::GetWindowWidth();
    float y = timeToScreenY(point->_time);
    return ImVec2(x, y) + ImGui::GetWindowPos();
}

AutomationPoint* AutomationWindow::screenPosToPoint(ImVec2& pos) {
    ImVec2 canvasPos = screenToCanvas(pos);
    double value = canvasPos.x / ImGui::GetWindowWidth();
    double time = toSnapRound(canvasPos.y);
    return new AutomationPoint(value, time);
}

void AutomationWindow::renderHeader() {
    if (_lane->_automationTarget == nullptr) {
        return;
    }
    Param* param = _lane->_automationTarget->getParam();
    ImGuiStyle style = ImGui::GetStyle();
    ImGui::SetCursorPosX(style.WindowPadding.x + offsetLeft());
    ImGui::Text(param->getValueText(0).c_str());
    ImGui::SameLine();
    std::string valueText = param->getValueText(0.5);
    ImVec2 valueTextSize = ImGui::CalcTextSize(valueText.c_str());
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - valueTextSize.x) / 2);
    ImGui::Text(valueText.c_str());
    ImGui::SameLine();
    valueText = param->getValueText(1.0);
    valueTextSize = ImGui::CalcTextSize(valueText.c_str());
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - valueTextSize.x - style.ScrollbarSize - style.WindowPadding.x);
    ImGui::Text(valueText.c_str());

}

void AutomationWindow::renderPoints() {
    for (auto& point : _clip->_sequence->_items) {
        ImVec2 pos = pointToScreenPos((AutomationPoint*)point.get());
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddCircle(pos, 2, IM_COL32_WHITE);
    }
}
