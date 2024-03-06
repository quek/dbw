#include "TimelineMixin.h"
#include "Composer.h"
#include "Grid.h"
#include "GuiUtil.h"

constexpr float GRID_SKIP_WIDTH = 20.0f;

TimelineMixin::TimelineMixin(Composer* composer) : _composer(composer) {
}

void TimelineMixin::renderTimeline() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();
    float scrollX = ImGui::GetScrollX();
    float scrollY = ImGui::GetScrollY();
    float windowHeight = ImGui::GetWindowHeight();
    float leftPadding = 2.0f;
    float lastY = -GRID_SKIP_WIDTH;
    ImVec2 clipRectMin = windowPos + ImVec2(0.0f, offsetTop());
    ImVec2 clipRectMax = ImVec2(windowPos.x + ImGui::GetWindowWidth(), windowPos.y + windowHeight);

    ImGui::PushClipRect(clipRectMin, clipRectMax, true);
    for (int i = 0; ; ++i) {
        float y = (i * 4 * _zoomY) + offsetTop() + offsetStart();
        if (y - lastY < GRID_SKIP_WIDTH) {
            continue;
        }
        ImVec2 pos = ImVec2(scrollX + leftPadding, y);
        ImVec2 pos1 = pos + ImVec2(-scrollX, -scrollY) + windowPos;
        if (pos1.y < clipRectMin.y - windowHeight) {
            continue;
        }
        if (clipRectMax.y < pos1.y) {
            ImGui::SetCursorPos(pos + ImVec2(0, windowHeight));
            ImGui::Text(" ");
            break;
        }
        ImGui::SetCursorPos(pos);
        ImGui::Text(std::to_string(i + 1).c_str());

        ImVec2 pos2 = pos + ImVec2(ImGui::GetWindowWidth() - scrollX, -scrollY) + windowPos;
        drawList->AddLine(pos1, pos2, BAR_LINE_COLOR);

        if (_zoomY >= GRID_SKIP_WIDTH) {
            renderGridBeat16th(drawList, pos1.y, pos1.x, pos2.x);
            for (int beat = 1; beat < 4; ++beat) {
                float beatY = pos1.y + (beat * _zoomY);
                drawList->AddLine(ImVec2(pos1.x + offsetLeft(), beatY), ImVec2(pos2.x, beatY), BEAT_LINE_COLOR);

                renderGridBeat16th(drawList, beatY, pos1.x, pos2.x);
            }
        } else if (2 * _zoomY >= GRID_SKIP_WIDTH) {
            float beatY = pos1.y + (2 * _zoomY);
            drawList->AddLine(ImVec2(pos1.x + offsetLeft(), beatY), ImVec2(pos2.x, beatY), BEAT_LINE_COLOR);
        }

        lastY = y;
    }
    {
        float x1 = windowPos.x;
        float y1 = timeToScreenY(_composer->_loopStartTime);
        ImVec2 pos1(x1, y1);
        float x2 = x1 + offsetLeft();
        float y2 = timeToScreenY(_composer->_loopEndTime);
        ImVec2 pos2(x2, y2);
        drawList->AddRectFilled(pos1, pos2, IM_COL32(0xcc, 0xcc, 0xcc, 0x80));
    }
    ImGui::PopClipRect();

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mousePos = io.MousePos;

    if (!canHandleInput() ||
        !Bounds(clipRectMin, clipRectMax - ImVec2(ImGui::GetWindowWidth() - offsetLeft(), 0.0f)).contains(mousePos)) {
        return;
    }
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        double time = timeFromMousePos();
        handleClickTimeline(time, io.KeyCtrl, io.KeyAlt);
    }
}

void TimelineMixin::renderGridBeat16th(ImDrawList* drawList, float beatY, float x1, float x2) {
    if (4 * _zoomY >= GRID_SKIP_WIDTH) {
        for (int beat16th = 1; beat16th < 4; ++beat16th) {
            float beat16thY = beatY + (1.0f / 4.0f * beat16th * _zoomY);
            drawList->AddLine(ImVec2(x1 + offsetLeft(), beat16thY), ImVec2(x2, beat16thY), BEAT16TH_LINE_COLOR, 1.0f);
        }
    }
}

ImVec2 TimelineMixin::screenToCanvas(const ImVec2& pos) {
    ImVec2 windowPos = ImGui::GetWindowPos();
    float scrollX = ImGui::GetScrollX();
    float scrollY = ImGui::GetScrollY();
    float x = (pos.x - windowPos.x - offsetLeft() + scrollX) / _zoomX;
    float y = (pos.y - windowPos.y - offsetTop() + scrollY - offsetStart()) / _zoomY;
    return ImVec2(x, y);
}

ImVec2 TimelineMixin::canvasToScreen(const ImVec2& pos) {
    ImVec2 windowPos = ImGui::GetWindowPos();
    float scrollX = ImGui::GetScrollX();
    float scrollY = ImGui::GetScrollY();
    float x = pos.x * _zoomX + windowPos.x + offsetLeft() - scrollX;
    float y = pos.y * _zoomY + windowPos.y + offsetTop() - scrollY + offsetStart();
    return ImVec2(x, y);
}

double TimelineMixin::timeFromMousePos(float offset, bool floor) {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mousePos = io.MousePos - ImVec2(0.0f, offset);
    ImVec2 canvasPos = screenToCanvas(mousePos);
    if (floor) {
        return toSnapFloor(canvasPos.y);
    }
    return toSnapRound(canvasPos.y);
}

float TimelineMixin::timeToScreenY(double time) {
    float scrollY = ImGui::GetScrollY();
    float y = time * _zoomY - scrollY + offsetTop() + offsetStart() + ImGui::GetWindowPos().y;
    return y;
}

double TimelineMixin::toSnapFloor(const double time) {
    if (!_snap) {
        return time;
    }
    return _grid->snapFloor(time);
}

double TimelineMixin::toSnapRound(const double time) {
    if (!_snap) {
        return time;
    }
    return _grid->snapRound(time);
}

