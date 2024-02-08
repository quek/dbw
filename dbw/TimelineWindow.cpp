#include "TimelineWindow.h"
#include <imgui.h>
#include "Composer.h"
#include "Grid.h"
#include "GuiUtil.h"
#include "TrackLane.h"

constexpr float GRID_SKIP_WIDTH = 20.0f;
constexpr float TIMELINE_START_OFFSET = 10.0f;
constexpr float TIMELINE_WIDTH = 15.0f;
constexpr float TRACK_HEADER_HEIGHT = 20.0f;

TimelineWindow::TimelineWindow(Composer* composer) : ZoomMixin(1.0f, 10.0f), _composer(composer) {
    _grid = gGrids[0].get();
}

void TimelineWindow::render() {
    ImGui::SetNextWindowSizeConstraints(ImVec2(300, 200), ImVec2(FLT_MAX, FLT_MAX));
    auto windowName = _composer->_project->_name.string() + "##Timeline";
    if (ImGui::Begin(windowName.c_str(), nullptr, ImGuiWindowFlags_NoScrollbar)) {
        renderGridSnap();
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        if (ImGui::BeginChild("##TimelineWindow Timeline",
                              ImVec2(0.0f, -22.0f),
                              ImGuiChildFlags_None,
                              ImGuiWindowFlags_HorizontalScrollbar)) {
            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 windowSize = ImGui::GetWindowSize();
            ImVec2 clipRectMin = windowPos;
            ImVec2 clipRectMax = clipRectMin + ImGui::GetWindowSize();
            ImGui::PushClipRect(clipRectMin, clipRectMax, true);

            renderTimeline();
            renderTrackHeader();
            ImGui::PopClipRect();

            clipRectMin += ImVec2(TIMELINE_WIDTH, TRACK_HEADER_HEIGHT);
            ImGui::PushClipRect(clipRectMin, clipRectMax, true);
            renderClips(windowPos);
            handleCanvas(clipRectMin, clipRectMax);
            ImGui::PopClipRect();
        }
        ImGui::EndChild();
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImGui::PopStyleVar();

        // TODO マウスホイールとかでスクロールするようにする
        renderDebugZoomSlider();
    }
    ImGui::End();
}

void TimelineWindow::handleCanvas(ImVec2& clipRectMin, ImVec2& clipRectMax) {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mousePos = io.MousePos;
    if (!Bounds(clipRectMin, clipRectMax).contains(mousePos)) {
        return;
    }
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();
    if (!(windowPos <= mousePos && mousePos < windowPos + windowSize)) {
        return;
    }

    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        float scrollX = ImGui::GetScrollX();
        float scrollY = ImGui::GetScrollY();
        float time = (mousePos.y - windowPos.y + scrollY - TRACK_HEADER_HEIGHT) / _zoomY;
        time = _grid->snapFloor(time);

        float x = (mousePos.x - windowPos.x + scrollX - TIMELINE_WIDTH) / _zoomX;
        TrackLane* lane = nullptr;
        for (auto& track : _composer->_tracks) {
            float trackWidth = getTrackWidth(track.get());
            if (trackWidth > x) {
                // TODO 複数レーン対応
                lane = track->_trackLanes[0].get();
                break;
            }
            x -= trackWidth;
        }
        if (lane != nullptr) {
            lane->_clips.emplace_back(new Clip(time));
        }
    }
}

void TimelineWindow::renderTimeline() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();
    float scrollX = ImGui::GetScrollX();
    float scrollY = ImGui::GetScrollY();
    float leftPadding = 2.0f;
    float lastY = -GRID_SKIP_WIDTH;
    float lineX = allTracksWidth() * _zoomX + TIMELINE_WIDTH;
    ImVec2 clipRectMin = windowPos + ImVec2(0.0f, TRACK_HEADER_HEIGHT);
    ImVec2 clipRectMax = ImVec2(windowPos.x + ImGui::GetWindowWidth(), windowPos.y + ImGui::GetWindowHeight());
    ImGui::PushClipRect(clipRectMin, clipRectMax, true);
    for (int i = 0; i < _composer->maxBar(); ++i) {
        float y = (4 * i * _zoomY) + TRACK_HEADER_HEIGHT;
        if (y - lastY < GRID_SKIP_WIDTH) {
            continue;
        }
        ImVec2 pos = ImVec2(scrollX + leftPadding, y);
        ImGui::SetCursorPos(pos);
        ImGui::Text(std::to_string(i + 1).c_str());

        ImVec2 pos1 = pos + ImVec2(-scrollX, -scrollY) + windowPos;
        ImVec2 pos2 = pos + ImVec2(lineX - scrollX, -scrollY) + windowPos;
        drawList->AddLine(pos1, pos2, BAR_LINE_COLOR);

        lastY = y;
    }
    ImGui::PopClipRect();
}

void TimelineWindow::renderTrackHeader() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();
    float scrollX = ImGui::GetScrollX();
    float scrollY = ImGui::GetScrollY();
    float x = TIMELINE_WIDTH;
    float y = ImGui::GetScrollY();
    ImGuiStyle& style = ImGui::GetStyle();
    float padding = style.FramePadding.x;
    ImVec2 clipRectMin = windowPos + ImVec2(TIMELINE_WIDTH, 0.0f);
    ImVec2 clipRectMax = ImVec2(windowPos.x + ImGui::GetWindowWidth(), windowPos.y + ImGui::GetWindowHeight());
    ImGui::PushClipRect(clipRectMin, clipRectMax, true);
    for (auto& track : _composer->_tracks) {
        ImVec2 pos = ImVec2(x, y);
        ImGui::SetCursorPos(pos + ImVec2(padding, 0));
        ImGui::Text(track->_name.c_str());

        ImVec2 pos1 = pos + ImVec2(-scrollX, -scrollY) + windowPos;
        ImVec2 pos2 = pos1 + ImVec2(0, _composer->maxBar() * 4 * _zoomY);
        drawList->AddLine(pos1, pos2, BAR_LINE_COLOR);

        x += getTrackWidth(track.get()) * _zoomX;
    }
    ImVec2 pos = ImVec2(x, y);
    ImGui::SetCursorPos(pos + ImVec2(padding, 0));
    ImGui::Text(_composer->_masterTrack->_name.c_str());
    ImVec2 pos1 = pos + ImVec2(-scrollX, -scrollY) + windowPos;
    ImVec2 pos2 = pos1 + ImVec2(0, _composer->maxBar() * 4 * _zoomY);
    drawList->AddLine(pos1, pos2, BAR_LINE_COLOR);
    ImGui::PopClipRect();
}

void TimelineWindow::renderClips(ImVec2& windowPos) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    float scrollX = ImGui::GetScrollX();
    float scrollY = ImGui::GetScrollY();
    float x = TIMELINE_WIDTH + windowPos.x;
    for (auto& track : _composer->_tracks) {
        float trackWidth = getTrackWidth(track.get());
        for (auto& clip : track->_trackLanes[0]->_clips) {
            float y1 = clip->_time * _zoomY - scrollY + TRACK_HEADER_HEIGHT + windowPos.y;
            float y2 = y1 + clip->_duration * _zoomY;
            ImVec2 pos1 = ImVec2(x - scrollX + 2.0f, y1);
            ImVec2 pos2 = ImVec2(x - scrollX + trackWidth - 1.0f, y2);
            drawList->AddRectFilled(pos1, pos2, IM_COL32_WHITE);
        }
        x += trackWidth;
    }
}

float TimelineWindow::getTrackWidth(Track* track) {
    if (!_trackWidthMap.contains(track)) {
        _trackWidthMap[track] = 100.0f;
    }
    return _trackWidthMap[track];
}

float TimelineWindow::allTracksWidth() {
    float width = 0.0f;
    for (auto& track : _composer->_tracks) {
        width += getTrackWidth(track.get());
    }
    return width + getTrackWidth(_composer->_masterTrack.get());
}
