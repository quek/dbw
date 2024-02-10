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

TimelineWindow::TimelineWindow(Composer* composer) : ZoomMixin(1.0f, 10.0f), _composer(composer) {
    _grid = gGrids[0].get();
}

void TimelineWindow::render() {
    _state.reset();

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

void TimelineWindow::handleCanvas(ImVec2& clipRectMin, ImVec2& clipRectMax) {
    if (!ImGui::IsWindowFocused()) {
        return;
    }

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

    if (!_state._consumedDoubleClick) {
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            float scrollX = ImGui::GetScrollX();
            float scrollY = ImGui::GetScrollY();
            float time = (mousePos.y - windowPos.y + scrollY - TRACK_HEADER_HEIGHT) / _zoomY;
            time = _grid->snapFloor(time);

            TrackLane* lane = laneFromMousePos();
            if (lane != nullptr) {
                lane->_clips.emplace_back(new Clip(time));
            }
        }
    }
    if (_state._draggingClip) {
        if (_state._clipClickedPart == Middle) {
            float time = clipTimeFromMouserPos(_state._clipClickedOffset);
            float timeDelta = time - _state._draggingClip->_time;
            TrackLane* lane = laneFromMousePos();
            // TODO laneDelta pointer?
            //int16_t key = noteKeyFromMouserPos();
            //int16_t keyDelta = key - _state._draggingClip->_key;
            _state._draggingClip->_time = time;
            //_state._draggingClip->_key = key;
            for (auto note : _state._selectedClips) {
                if (note != _state._draggingClip) {
                    note->_time += timeDelta;
                    // TODO lane note->_key += keyDelta;
                }
            }
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
        } else if (_state._clipClickedPart == Top) {
            float time = clipTimeFromMouserPos();
            float timeDelta = time - _state._draggingClip->_time;
            if (std::ranges::all_of(_state._selectedClips,
                                    [timeDelta](auto x) { return x->_duration > timeDelta; })) {
                for (auto clip : _state._selectedClips) {
                    clip->_time += timeDelta;
                    clip->_duration += -timeDelta;
                }
            }
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        } else if (_state._clipClickedPart == Bottom) {
            float time = clipTimeFromMouserPos();
            float timeDelta = time - _state._draggingClip->_time - _state._draggingClip->_duration;
            if (std::ranges::all_of(_state._selectedClips,
                                    [timeDelta](auto x) { return x->_duration > -timeDelta; })) {
                for (auto clip : _state._selectedClips) {
                    clip->_duration += timeDelta;
                }
            }
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            // ノートのドラッグ解除
            _state._draggingClip = nullptr;
        }
    } else if (_state._rangeSelecting) {
        // Ctrl でトグル、Shift で追加 Reason の仕様がいいと思う
        ImVec2 pos1 = io.MouseClickedPos[ImGuiMouseButton_Left];
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddRect(pos1, mousePos, RANGE_SELECTING_COLOR, 0.0f, ImDrawFlags_None, 3.0f);
        if (!io.KeyShift && !io.KeyCtrl) {
            _state._selectedClips.clear();
        }
        Bounds bounds(pos1, mousePos);
        for (auto& track : _composer->_tracks) {
            for (auto& clip : track->_trackLanes[0]->_clips) {
                if (bounds.overlaped(_clipBoundsMap[clip.get()])) {
                    if (io.KeyCtrl) {
                        if (_state._selectedClips.contains(clip.get())) {
                            _state._selectedClips.erase(clip.get());
                        } else {
                            _state._selectedClips.insert(clip.get());
                        }
                    } else {
                        _state._selectedClips.insert(clip.get());
                    }
                }
            }
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            // 範囲選択解除
            _state._rangeSelecting = false;
        }
    } else {
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.1f)) {
            // ここがノートのドラッグの開始
            if (!_state._selectedClips.empty() && _state._clickedClip) {
                // ノートの移動 or 長さ変更
                _state._draggingClip = _state._clickedClip;
            } else {
                // 範囲選択
                _state._rangeSelecting = true;
            }
        } else if (!_state._consumedClicked) {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !io.KeyCtrl && !io.KeyShift) {
                _state._selectedClips.clear();
            }
            if (!ImGui::IsAnyMouseDown()) {
                _state._clickedClip = nullptr;
            }
        }
    }
}

void TimelineWindow::handleShortcut() {
    if (!ImGui::IsWindowFocused()) {
        return;
    }
    ImGuiIO& io = ImGui::GetIO();
    if (io.KeysDown[ImGuiKey_Delete] || io.KeyCtrl && io.KeysDown[ImGuiKey_D]) {
        _composer->deleteClips(_state._selectedClips);
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

void TimelineWindow::renderClips(ImVec2& windowPos) {
    _clipBoundsMap.clear();
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mousePos = io.MousePos;
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    float scrollX = ImGui::GetScrollX();
    float scrollY = ImGui::GetScrollY();
    float x = TIMELINE_WIDTH + windowPos.x;
    for (auto& track : _composer->_tracks) {
        float trackWidth = getTrackWidth(track.get());
        for (const auto& clip : track->_trackLanes[0]->_clips) {
            float y1 = clip->_time * _zoomY - scrollY + TRACK_HEADER_HEIGHT + windowPos.y;
            float y2 = y1 + clip->_duration * _zoomY;
            ImVec2 pos1 = ImVec2(x - scrollX + 2.0f, y1);
            ImVec2 pos2 = ImVec2(x - scrollX + trackWidth - 1.0f, y2);
            ImU32 clipColor;
            if (_state._selectedClips.contains(clip.get())) {
                clipColor = SELECTED_CLIP_COLOR;
            } else {
                clipColor = CLIP_COLOR;
            }
            drawList->AddRectFilled(pos1, pos2, clipColor);
            _clipBoundsMap[clip.get()] = Bounds(pos1, pos2);

            if (!ImGui::IsWindowFocused() || _state._draggingClip || !Bounds(pos1, pos2).contains(mousePos)) {
                continue;
            }

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                _state._unselectClickedClipIfMouserReleased = io.KeyCtrl && _state._selectedClips.contains(clip.get());
                if (!io.KeyCtrl && !_state._selectedClips.contains(clip.get())) {
                    _state._selectedClips.clear();
                }
                _state._selectedClips.insert(clip.get());
                _state._clickedClip = clip.get();
                _state._consumedClicked = true;
            } else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                if (_state._unselectClickedClipIfMouserReleased) {
                    _state._selectedClips.erase(_state._clickedClip);
                }
                _state._unselectClickedClipIfMouserReleased = false;
            }

            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                _composer->_pianoRoll->edit(clip.get());
                _state._consumedDoubleClick = true;
            }

            if (mousePos.y - pos1.y <= 5) {
                _state._clipClickedPart = Top;
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
            } else if (pos2.y - mousePos.y <= 5) {
                _state._clipClickedPart = Bottom;
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
            } else {
                _state._clipClickedPart = Middle;
                _state._clipClickedOffset = mousePos.y - pos1.y;
            }
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

ImVec2 TimelineWindow::toCanvasPos(ImVec2& pos) const {
    ImVec2 windowPos = ImGui::GetWindowPos();
    float scrollX = ImGui::GetScrollX();
    float scrollY = ImGui::GetScrollY();
    float x = (pos.x - windowPos.x - TIMELINE_WIDTH - TIMELINE_START_OFFSET + scrollX) / _zoomX;
    float y = (pos.y - windowPos.y - TRACK_HEADER_HEIGHT + scrollY) / _zoomY;
    return ImVec2(x, y);
}

double TimelineWindow::clipTimeFromMouserPos(float offset) {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mousePos = io.MousePos - ImVec2(offset, 0.0f);
    ImVec2 canvasPos = toCanvasPos(mousePos);
    double time = toSnapRound(canvasPos.y);
    return time;
}

double TimelineWindow::toSnapFloor(const double time) {
    if (!_snap) {
        return time;
    }
    return _grid->snapFloor(time);
}

double TimelineWindow::toSnapRound(const double time) {
    if (!_snap) {
        return time;
    }
    return _grid->snapRound(time);
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

void TimelineWindow::State::reset() {
    _consumedDoubleClick = false;
    _consumedClicked = false;
}
