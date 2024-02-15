#include "TimelineWindow.h"
#include <mutex>
#include <imgui.h>
#include "AudioEngine.h"
#include "Composer.h"
#include "Grid.h"
#include "GuiUtil.h"
#include "TrackLane.h"

constexpr float TIMELINE_START_OFFSET = 10.0f;
constexpr float TIMELINE_WIDTH = 20.0f;
constexpr float TRACK_HEADER_HEIGHT = 20.0f;

ImU32 CLIP_COLOR = IM_COL32(0x00, 0xcc, 0xcc, 0x88);
ImU32 SELECTED_CLIP_COLOR = IM_COL32(0x66, 0x66, 0xff, 0x88);

TimelineWindow::TimelineWindow(Composer* composer) : TimelineCanvasMixin(composer) {
    _zoomX = 1.0f;
    _zoomY = 10.0f;
    _grid = gGrids[0].get();
}

void TimelineWindow::handleMove(double oldTime, double newTime, TrackLane* oldLane, TrackLane* newLane) {
    double timeDelta = newTime - oldTime;

    auto oldIndex = std::distance(_allLanes.begin(), std::find(_allLanes.begin(), _allLanes.end(), oldLane));
    auto newIndex = std::distance(_allLanes.begin(), std::find(_allLanes.begin(), _allLanes.end(), newLane));
    auto indexDelta = newIndex - oldIndex;

    for (auto clip : _state._selectedThings) {
        clip->_time += timeDelta;

        if (indexDelta == 0) {
            continue;
        }
        TrackLane* from = _clipLaneMap[clip];
        auto fromIndex = std::distance(_allLanes.begin(), std::find(_allLanes.begin(), _allLanes.end(), from));
        auto toIndex = fromIndex + indexDelta;
        if (toIndex < 0 || toIndex >= static_cast<int>(_allLanes.size())) {
            continue;
        }
        TrackLane* to = _allLanes[toIndex];
        auto it = std::ranges::find_if(from->_clips, [clip](const auto& x) { return x.get() == clip; });
        to->_clips.push_back(std::move(*it));
        from->_clips.erase(it);
    }
}

void TimelineWindow::handleMouse(const ImVec2& clipRectMin, const ImVec2& clipRectMax) {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mousePos = io.MousePos;
    if (!Bounds(clipRectMin, clipRectMax).contains(mousePos)) {
        return;
    }
    const ImGuiPayload* payload = ImGui::GetDragDropPayload();
    if (payload && payload->IsDataType("Sequence Matrix Clip")) {
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            Clip* clip = new Clip(*(const Clip*)payload->Data);
            float time = timeFromMousePos(0.0f, false);
            TrackLane* lane = laneFromPos(mousePos);
            clip->_time = time;
            lane->_clips.emplace_back(clip);
        }
    }

    TimelineCanvasMixin::handleMouse(clipRectMin, clipRectMax);
}

void TimelineWindow::handleClickTimeline(double time) {
    std::lock_guard<std::mutex> lock(_composer->_audioEngine->mtx);
    _composer->_playTime = time;
}

Clip* TimelineWindow::copyThing(Clip* other) {
    Clip* clip = new Clip(*other);
    _clipLaneMap[other]->_clips.emplace_back(clip);
    return clip;
}

void TimelineWindow::deleteThing(Clip* clip) {
    auto lane = _clipLaneMap[clip];
    auto it = std::ranges::find_if(lane->_clips,
                                   [clip](const auto& x) { return x.get() == clip; });
    if (it != lane->_clips.end()) {
        lane->_clips.erase(it);
    }
}

void TimelineWindow::handleDoubleClick(Clip* clip) {
    _composer->_pianoRollWindow->edit(clip);
}

Clip* TimelineWindow::handleDoubleClick(double time, TrackLane* lane) {
    // TODO undo
    Clip* clip = new Clip(time);
    lane->_clips.emplace_back(clip);
    return clip;
}

void TimelineWindow::prepareAllThings() {
    _allThings.clear();
    _allLanes.clear();
    _clipLaneMap.clear();
    for (auto& track : _composer->_tracks) {
        for (auto& lane : track->_trackLanes) {
            _allLanes.push_back(lane.get());
            for (auto& clip : lane->_clips) {
                _allThings.push_back(clip.get());
                _clipLaneMap[clip.get()] = lane.get();
            }
        }
    }
}

void TimelineWindow::renderThing(Clip* clip, const ImVec2& pos1, const ImVec2& pos2) {
    TimelineCanvasMixin::renderThing(clip, pos1, pos2);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddText(pos1, IM_COL32_WHITE, clip->name().c_str());
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
    for (auto it : _allLanes) {
        if (it == lane) {
            return x;
        }
        x += getLaneWidth(it);
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

void TimelineWindow::renderPalyhead() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();
    float scrollY = ImGui::GetScrollY();
    float y = (_composer->_playTime * _zoomY) + offsetTop() + offsetStart();
    if (_composer->_playing && _composer->_isScrollFolloPlayhead) {
        if (y < ImGui::GetWindowHeight() / 2.0f) {
            ImGui::SetScrollY(0);
        } else {
            ImGui::SetScrollY(y - ImGui::GetWindowHeight() / 2.0f);
        }
        y = std::min(y, ImGui::GetWindowHeight() / 2.0f);
    } else {
        y -= scrollY;
    }
    ImVec2 pos1 = ImVec2(0.0f, y) + windowPos;
    ImVec2 pos2 = ImVec2(ImGui::GetWindowWidth(), y) + windowPos;
    drawList->AddLine(pos1, pos2, PLAY_CURSOR_COLOR);
}

void TimelineWindow::renderHeader() {
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

std::string TimelineWindow::windowName() {
    return _composer->_project->_name.string() + "##Timeline";
}

std::string TimelineWindow::canvasName() {
    return "##Timeline Canvas";
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

TrackLane* TimelineWindow::laneFromPos(ImVec2& pos) {
    ImVec2 windowPos = ImGui::GetWindowPos();
    float scrollX = ImGui::GetScrollX();
    float x = (pos.x - windowPos.x + scrollX - TIMELINE_WIDTH) / _zoomX;
    for (auto& track : _composer->_tracks) {
        for (auto& lane : track->_trackLanes) {
            float laneWidth = getLaneWidth(lane.get());
            if (laneWidth > x) {
                return lane.get();
            }
            x -= laneWidth;
        }
    }
    return nullptr;
}
