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

ImU32 CLIP_COLOR = IM_COL32(0x00, 0xcc, 0xcc, 0x88);
ImU32 SELECTED_CLIP_COLOR = IM_COL32(0x66, 0x66, 0xff, 0x88);

TimelineWindow::TimelineWindow(Composer* composer) : TimelineCanvasMixin(composer) {
    _zoomX = 1.0f;
    _zoomY = 10.0f;
    _grid = gGrids[0].get();
}

void TimelineWindow::render() {
    _state.reset();
    prepareAllThings();

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
            renderThing(windowPos);
            handleMouse(clipRectMin, clipRectMax);
            handleShortcut();
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

void TimelineWindow::handleDoubleClick(Clip* clip) {
    _composer->_pianoRoll->edit(clip);
}

void TimelineWindow::handleDoubleClick(double time, TrackLane* lane) {
    // TODO undo
    lane->_clips.emplace_back(new Clip(time));
}

void TimelineWindow::prepareAllThings() {
    _allThings.clear();
    _clipLaneMap.clear();
    for (auto& track : _composer->_tracks) {
        for (auto& lane : track->_trackLanes) {
            for (auto& clip : lane->_clips) {
                _allThings.push_back(clip.get());
                _clipLaneMap[clip.get()] = lane.get();
            }
        }
    }
}

float TimelineWindow::offsetTop() const {
    return TRACK_HEADER_HEIGHT;
}

float TimelineWindow::offsetLeft() const {
    return TIMELINE_WIDTH;
}

float TimelineWindow::offsetStart() const {
    return TIMELINE_START_OFFSET;
}

ImU32 TimelineWindow::colorSlectedThing() {
    return SELECTED_CLIP_COLOR;
}

ImU32 TimelineWindow::colorThing() {
    return CLIP_COLOR;
}

float TimelineWindow::xFromThing(Clip* clip) {
    TrackLane* lane = _clipLaneMap[clip];
    float x = 0.0f;
    for (auto& track : _composer->_tracks) {
        for (auto& laneItr : track->_trackLanes) {
            if (laneItr.get() == lane) {
                return x;
            }
            x += getLaneWidth(laneItr.get());
        }
    }
    return x;
}

void TimelineWindow::handleShortcut() {
    if (!ImGui::IsWindowFocused()) {
        return;
    }
    ImGuiIO& io = ImGui::GetIO();
    if (io.KeysDown[ImGuiKey_Delete] || io.KeyCtrl && io.KeysDown[ImGuiKey_D]) {
        _composer->deleteClips(_state._selectedThings);
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
        float y = (i * 4 * _zoomY) + TRACK_HEADER_HEIGHT;
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

    float y = (_composer->_playTime * 4 * _zoomY) + TRACK_HEADER_HEIGHT;
    ImVec2 pos1 = ImVec2(0.0f, y - scrollY) + windowPos;
    ImVec2 pos2 = ImVec2(lineX, y - scrollY) + windowPos;
    drawList->AddLine(pos1, pos2, PLAY_CURSOR_COLOR);

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

float TimelineWindow::getLaneWidth(Clip* clip) {
    TrackLane* lane = _clipLaneMap[clip];
    return getLaneWidth(lane);
}

float TimelineWindow::getLaneWidth(TrackLane* lane) {
    if (!_laneWidthMap.contains(lane)) {
        _laneWidthMap[lane] = 100.0f;
    }
    return _laneWidthMap[lane];
}

float TimelineWindow::getTrackWidth(Track* track) {
    float width = 0.0f;
    for (auto& lane : track->_trackLanes) {
        width += getLaneWidth(lane.get());
    }
    return width;
}

float TimelineWindow::allTracksWidth() {
    float width = 0.0f;
    for (auto& track : _composer->_tracks) {
        width += getTrackWidth(track.get());
    }
    return width + getTrackWidth(_composer->_masterTrack.get());
}

TrackLane* TimelineWindow::laneFromMousePos() {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mousePos = io.MousePos;
    ImVec2 windowPos = ImGui::GetWindowPos();
    float scrollX = ImGui::GetScrollX();
    float x = (mousePos.x - windowPos.x + scrollX - TIMELINE_WIDTH) / _zoomX;
    TrackLane* lane = nullptr;
    for (auto& track : _composer->_tracks) {
        float trackWidth = getTrackWidth(track.get());
        if (trackWidth > x) {
            // TODO 複数レーン対応
            lane = track->_trackLanes[0].get();
            return lane;
        }
        x -= trackWidth;
    }
    return nullptr;
}
