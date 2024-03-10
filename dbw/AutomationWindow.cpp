#include "AutomationWindow.h"
#include "AutomationClip.h"
#include "Composer.h"
#include "Config.h"
#include "Grid.h"
#include "Lane.h"

constexpr float POINT_RADIUS = 4.0f;

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
            renderPlayhead();
            renderPoints();
            handleMouse();
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();

        handleShortcut();
    }
    ImGui::End();
}

float AutomationWindow::getCanvasWidth() {
    auto& style = ImGui::GetStyle();
    float width = ImGui::GetWindowWidth() - offsetLeft() - POINT_RADIUS * 2 - style.ScrollbarSize;
    return width;
}

void AutomationWindow::handleMouse() {
    //if (!canHandleInput()) {
    //    return;
    //}

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mousePos = io.MousePos;
    std::unique_ptr<AutomationPoint> point(screenPosToPoint(mousePos));

    if (_draggingPoint) {
        // TODO command
        _draggingPoint->setValue(point->getValue());
        _draggingPoint->setTime(point->getTime());
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            _draggingPoint = nullptr;
        }
    } else if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        if (_targetPoint) {
            // point のドラッグ開始
            _draggingPoint = _targetPoint;
        } else {
            // 範囲選択開始
            _rangeSelecting = true;

        }
    } else if (canHandleInput()) {
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            if (_pointAtMouse) {
                _clip->_sequence->deleteItem(_pointAtMouse);
            } else {
                _clip->_sequence->addItem(point.release());
            }
        } else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            if (_pointAtMouse) {
                _targetPoint = _pointAtMouse;
            } else {
                _targetPoint = nullptr;
            }
        }
    }
}

void AutomationWindow::handleShortcut() {
    if (defineShortcut(ImGuiKey_Escape)) {
        _show = false;
    }
}

ImVec2 AutomationWindow::pointToScreenPos(const AutomationPoint& point) {
    return pointToScreenPos(point.getValue(), point.getTime());
}

ImVec2 AutomationWindow::pointToScreenPos(double value, double time) {
    float x = value * getCanvasWidth() + ImGui::GetWindowPos().x + offsetLeft() + POINT_RADIUS;
    float y = timeToScreenY(time);
    return ImVec2(x, y);
}

AutomationPoint* AutomationWindow::screenPosToPoint(ImVec2& pos) {
    ImVec2 canvasPos = screenToCanvas(pos);
    double value = (canvasPos.x - POINT_RADIUS) / getCanvasWidth();
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

void AutomationWindow::renderPlayhead() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();
    float scrollY = ImGui::GetScrollY();
    float y = (_composer->_playTime * _zoomY) + offsetTop() + offsetStart() - scrollY;
    ImVec2 pos1 = ImVec2(0.0f, y) + windowPos;
    ImVec2 pos2 = ImVec2(ImGui::GetWindowWidth(), y) + windowPos;
    drawList->AddLine(pos1, pos2, PLAY_CURSOR_COLOR);
}

void AutomationWindow::renderPoints() {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mousePos = io.MousePos;
    _pointAtMouse = nullptr;

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    double defaultValue = _lane->_automationTarget->getDefaultValue();
    ImVec2 pos1 = pointToScreenPos(defaultValue, 0.0);
    bool first = true;
    for (auto& it : _clip->_sequence->getItems()) {
        AutomationPoint* point = (AutomationPoint*)it.get();
        ImVec2 pos2 = pointToScreenPos(*point);
        if (first) {
            first = false;
            pos1 = pointToScreenPos(point->getValue(), 0.0);
        }
        drawList->AddLine(pos1, pos2, gTheme.automationLine);
        if (point == _targetPoint) {
            drawList->AddCircleFilled(pos2, POINT_RADIUS, gTheme.automationPoint);
        } else {
            drawList->AddCircle(pos2, POINT_RADIUS, gTheme.automationPoint);
        }
        Bounds bounds(pos2 - ImVec2(POINT_RADIUS, POINT_RADIUS), pos2 + ImVec2(POINT_RADIUS, POINT_RADIUS));
        if (bounds.contains(mousePos)) {
            _pointAtMouse = point;
        }
        if (bounds.contains(mousePos) || point == _targetPoint) {
            Param* param = _lane->_automationTarget->getParam();
            std::string valueText = param->getValueText(point->getValue());
            drawList->AddText(pos2 + ImVec2((point->getValue() < 0.5) ? 15.0f : -(15.0f + ImGui::CalcTextSize(valueText.c_str()).x), 0.0f),
                              gTheme.text,
                              valueText.c_str());
        }
        pos1 = pos2;
    }
    ImVec2 pos2 = ImVec2(pos1.x, timeToScreenY(_clip->_sequence->getDuration()));
    drawList->AddLine(pos1, pos2, gTheme.automationLine);
}

