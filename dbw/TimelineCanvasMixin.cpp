#include "TimelineCanvasMixin.h"
#include <algorithm>
#include "Clip.h"
#include "Config.h"
#include "Composer.h"
#include "Grid.h"
#include "GuiUtil.h"
#include "Lane.h"

constexpr float GRID_SKIP_WIDTH = 20.0f;

template<class THING, typename LANE>
TimelineCanvasMixin<THING, LANE>::TimelineCanvasMixin(Composer* composer) : _composer(composer) {
}

template<class THING, typename LANE>
void TimelineCanvasMixin<THING, LANE>::render() {
    _state.reset();
    prepareAllThings();

    ImGui::SetNextWindowSizeConstraints(ImVec2(300, 200), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::Begin(windowName().c_str(), nullptr, ImGuiWindowFlags_NoScrollbar)) {
        renderGridSnap();

        ImGui::SameLine();
        ImGui::Checkbox("Follow Playhead", &_composer->_isScrollFolloPlayhead);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        if (ImGui::BeginChild(canvasName().c_str(),
                              ImVec2(0.0f, 0.0f),
                              ImGuiChildFlags_None,
                              ImGuiWindowFlags_HorizontalScrollbar)) {
            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 clipRectMin = windowPos;
            ImVec2 clipRectMax = clipRectMin + ImGui::GetWindowSize();

            ImGui::PushClipRect(clipRectMin + ImVec2(0.0f, offsetTop()), clipRectMax, true);
            renderTimeline();
            renderPlayhead();
            ImGui::PopClipRect();

            ImGui::PushClipRect(clipRectMin + ImVec2(offsetLeft(), 0.0f), clipRectMax, true);
            renderHeader();
            ImGui::PopClipRect();

            clipRectMin += ImVec2(offsetLeft(), offsetTop());
            ImGui::PushClipRect(clipRectMin, clipRectMax, true);
            renderThings();
            handleMouse(clipRectMin, clipRectMax);
            handleShortcut();
            ImGui::PopClipRect();

            renderEditCursor();
        }
        ImGui::EndChild();
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImGui::PopStyleVar();

        // DONE マウスホイールとかでスクロールするようにする
        // renderDebugZoomSlider();
    }
    ImGui::End();
}
template<class THING, typename LANE>
void TimelineCanvasMixin<THING, LANE>::handleMouse(const ImVec2& clipRectMin, const ImVec2& clipRectMax) {
    if (!canHandleInput()) {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mousePos = io.MousePos;
    if (!Bounds(clipRectMin, clipRectMax).contains(mousePos)) {
        return;
    }

    THING* thingAtMouse = thingAtPos(mousePos);

    if (_state._draggingThing) {
        if (_state._thingClickedPart == Middle) {
            double oldTime = _state._draggingThing->_time;
            double newTime = std::max(timeFromMousePos(_state._thingClickedOffset), 0.0);
            LANE* oldLane = laneFromPos(_state._thingBoundsMap[_state._draggingThing].p);
            LANE* newLane = laneFromPos(mousePos);
            handleMove(oldTime, newTime, oldLane, newLane);

            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
        } else if (_state._thingClickedPart == Top) {
            float time = timeFromMousePos();
            float timeDelta = time - _state._draggingThing->_time;
            if (std::ranges::all_of(_state._selectedThings,
                                    [timeDelta](auto x) { return x->_duration > timeDelta; })) {
                for (auto thing : _state._selectedThings) {
                    thing->_time += timeDelta;
                    thing->_duration += -timeDelta;
                }
            }
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
        } else if (_state._thingClickedPart == Bottom) {
            float time = timeFromMousePos();
            float timeDelta = time - _state._draggingThing->_time - _state._draggingThing->_duration;
            if (std::ranges::all_of(_state._selectedThings,
                                    [timeDelta](auto x) { return x->_duration > -timeDelta; })) {
                for (auto thing : _state._selectedThings) {
                    thing->_duration += timeDelta;
                }
            }
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            // ノートのドラッグ解除
            std::vector<Command*> commands;
            if (_state._thingClickedPart == Middle) {
                if (!io.KeyCtrl) {
                    commands.push_back(deleteThings(_state._selectedThings, true));
                }
                auto [copiedThings, copyCommand] = copyThings(_state._draggingThings, true);
                commands.push_back(copyCommand);
                _composer->_commandManager.executeCommand(commands, true);
                auto deleteCommand = deleteThings(_state._draggingThings, false);
                _composer->_commandManager.executeCommand(deleteCommand);
                _state._draggingThings = copiedThings;
            }
            _state._selectedThings = _state._draggingThings;
            _state._defaultThingDuration = _state._draggingThing->_duration;
            _state._draggingThing = nullptr;
            _state._draggingThings.clear();

        }
    } else if (_state._rangeSelecting) {
        // Ctrl でトグル、Shift で追加 Reason の仕様がいいと思う
        ImVec2 pos1 = io.MouseClickedPos[ImGuiMouseButton_Left];
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddRect(pos1, mousePos, RANGE_SELECTING_COLOR, 0.0f, ImDrawFlags_None, 3.0f);
        if (!io.KeyShift && !io.KeyCtrl) {
            _state._selectedThings.clear();
        }
        Bounds bounds(pos1, mousePos);
        std::set<THING*> inRangeThings;
        for (auto& thing : _allThings) {
            if (bounds.overlaped(_state._thingBoundsMap[thing])) {
                inRangeThings.insert(thing);
            }
        }
        if (io.KeyCtrl) {
            _state._selectedThings = _state._selectedThingsAtStartRangeSelecting;
            for (auto thing : inRangeThings) {
                if (_state._selectedThings.contains(thing)) {
                    _state._selectedThings.erase(thing);
                } else {
                    _state._selectedThings.insert(thing);
                }
            }
        } else if (io.KeyShift) {
            _state._selectedThings.insert(inRangeThings.begin(), inRangeThings.end());
        } else {
            _state._selectedThings = inRangeThings;
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            // 範囲選択解除
            _state._rangeSelecting = false;
        }
    } else if (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.1f)) {
        // ここがノートのドラッグの開始
        if (!_state._selectedThings.empty() && _state._clickedThing) {
            // ノートの移動 or 長さ変更
            _state._draggingThings.clear();
            if (_state._thingClickedPart == Middle) {
                std::set<THING*> x;
                x.insert(_state._clickedThing);
                auto [x1, c1] = copyThings(x, false);
                _state._draggingThing = *x1.begin();

                _state._selectedThings.erase(_state._clickedThing);
                auto [x2, c2] = copyThings(_state._selectedThings, false);
                _state._draggingThings = x2;
                _state._selectedThings.insert(_state._clickedThing);

                _state._draggingThings.insert(_state._draggingThing);

                _composer->_commandManager.executeCommand(c1);
                _composer->_commandManager.executeCommand(c2);
            } else {
                _state._draggingThing = _state._clickedThing;
                _state._draggingThings = _state._selectedThings;
            }
        } else {
            // 範囲選択
            _state._rangeSelecting = true;
            _state._selectedThingsAtStartRangeSelecting = _state._selectedThings;
        }
    } else if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        if (thingAtMouse) {
            handleDoubleClick(thingAtMouse);
        } else {
            float time = timeFromMousePos(0.0f, true);
            LANE* lane = laneFromPos(mousePos);
            THING* thing = handleDoubleClick(time, lane);
            _state._clickedThing = thing;
            _state._selectedThings.insert(thing);
            // マウス離さなかったらドラッグで長さを変えられる
            _state._thingClickedPart = Bottom;
        }
    } else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        if (thingAtMouse) {
            _state._unselectClickedThingIfMouserReleased = io.KeyCtrl && _state._selectedThings.contains(thingAtMouse);
            if (!io.KeyCtrl && !_state._selectedThings.contains(thingAtMouse)) {
                _state._selectedThings.clear();
            }
            _state._selectedThings.insert(thingAtMouse);
            _state._clickedThing = thingAtMouse;
            Bounds& bounds = _state._thingBoundsMap[thingAtMouse];
            if (mousePos.y - bounds.p.y <= 5) {
                _state._thingClickedPart = Top;
            } else if (bounds.q.y - mousePos.y <= 5) {
                _state._thingClickedPart = Bottom;
            } else {
                _state._thingClickedPart = Middle;
                _state._thingClickedOffset = mousePos.y - bounds.p.y;
            }
            _state._defaultThingDuration = thingAtMouse->_duration;
            onClickThing(thingAtMouse);
        } else {
            _state._clickedThing = nullptr;
            if (!io.KeyCtrl && !io.KeyShift) {
                _state._selectedThings.clear();
            }
        }
    } else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        if (_state._unselectClickedThingIfMouserReleased) {
            if (thingAtMouse == _state._clickedThing) {
                _state._selectedThings.erase(thingAtMouse);
            }
            _state._unselectClickedThingIfMouserReleased = false;
        } else if (!io.KeyCtrl && _state._selectedThings.size() > 1 && thingAtMouse == _state._clickedThing) {
            _state._selectedThings.clear();
            _state._selectedThings.insert(thingAtMouse);
        }
    } else if (thingAtMouse != nullptr) {
        Bounds& bounds = _state._thingBoundsMap[thingAtMouse];
        if (mousePos.y - bounds.p.y <= 5) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
        } else if (bounds.q.y - mousePos.y <= 5) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
        }
    }

    // マウスホイールでのズーム
    if (io.MouseWheel != 0.0f) {
        if (io.KeyCtrl) {
            // Alt だけで横ズームしたいけど縦スクロールしちゃうので仕方なく
            if (io.KeyAlt) {
                _zoomX += io.MouseWheel * 0.05f;
                if (_zoomX <= 0.1f) {
                    _zoomX = 0.1f;
                }
            } else {
                _zoomY += io.MouseWheel * _zoomY * 0.1f;
                if (_zoomY <= 0.1f) {
                    _zoomY = 0.1f;
                }
            }
        }
    }

    _state._editCursorPos = ImVec2(laneToScreenX(laneFromPos(mousePos)),
                                   timeToScreenY(timeFromMousePos(0, true)));
}

template<class THING, typename LANE>
void TimelineCanvasMixin<THING, LANE>::renderThings() {
    _state._thingBoundsMap.clear();
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mousePos = io.MousePos;
    for (const auto& thing : _allThings) {
        float x1 = xFromThing(thing);
        float x2 = x1 + getLaneWidth(thing) * _zoomX;
        float y1 = timeToScreenY(thing->_time);
        float y2 = y1 + thing->_duration * _zoomY;
        ImVec2 pos1 = ImVec2(x1 + 2.0f, y1);
        ImVec2 pos2 = ImVec2(x2 - 1.0f, y2);
        renderThing(thing, pos1, pos2);
        _state._thingBoundsMap[thing] = Bounds(pos1, pos2);
    }
}

template<class THING, typename LANE>
void TimelineCanvasMixin<THING, LANE>::renderThing(THING* thing, const ImVec2& pos1, const ImVec2& pos2) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImU32 thingColor;
    if (_state._selectedThings.contains(thing)) {
        thingColor = colorSlectedThing();
    } else {
        thingColor = colorThing();
    }
    drawList->AddRectFilled(pos1, pos2, thingColor);
}

template<class THING, typename LANE>
void TimelineCanvasMixin<THING, LANE>::renderTimeline() {
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

template<class THING, typename LANE>
void TimelineCanvasMixin<THING, LANE>::renderGridBeat16th(ImDrawList* drawList, float beatY, float x1, float x2) {
    if (4 * _zoomY >= GRID_SKIP_WIDTH) {
        for (int beat16th = 1; beat16th < 4; ++beat16th) {
            float beat16thY = beatY + (1.0f / 4.0f * beat16th * _zoomY);
            drawList->AddLine(ImVec2(x1 + offsetLeft(), beat16thY), ImVec2(x2, beat16thY), BEAT16TH_LINE_COLOR, 1.0f);
        }
    }
}

template<class THING, typename LANE>
void TimelineCanvasMixin<THING, LANE>::renderEditCursor() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 pos1 = ImVec2(windowPos.x, _state._editCursorPos.y);
    ImVec2 pos2 = pos1 + ImVec2(ImGui::GetWindowWidth(), 0.0f);
    drawList->AddLine(pos1, pos2, gTheme.editCursor);

    pos1 = ImVec2(_state._editCursorPos.x, windowPos.y);
    pos2 = pos1 + ImVec2(0.0f, ImGui::GetWindowHeight());
    drawList->AddLine(pos1, pos2, gTheme.editCursor);
}

template<class THING, typename LANE>
double TimelineCanvasMixin<THING, LANE>::timeFromMousePos(float offset, bool floor) {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mousePos = io.MousePos - ImVec2(0.0f, offset);
    ImVec2 canvasPos = screenToCanvas(mousePos);
    if (floor) {
        return toSnapFloor(canvasPos.y);
    }
    return toSnapRound(canvasPos.y);
}

template<class THING, typename LANE>
THING* TimelineCanvasMixin<THING, LANE>::thingAtPos(ImVec2& pos) {
    auto thing = std::ranges::find_if(_allThings, [&](auto x) { return _state._thingBoundsMap[x].contains(pos); });
    if (thing != _allThings.end()) {
        return *thing;
    }
    return nullptr;
}

template<class THING, typename LANE>
float TimelineCanvasMixin<THING, LANE>::timeToScreenY(double time) {
    float scrollY = ImGui::GetScrollY();
    float y = time * _zoomY - scrollY + offsetTop() + offsetStart() + ImGui::GetWindowPos().y;
    return y;
}

template<class THING, typename LANE>
double TimelineCanvasMixin<THING, LANE>::toSnapFloor(const double time) {
    if (!_snap) {
        return time;
    }
    return _grid->snapFloor(time);
}

template<class THING, typename LANE>
double TimelineCanvasMixin<THING, LANE>::toSnapRound(const double time) {
    if (!_snap) {
        return time;
    }
    return _grid->snapRound(time);
}

template<class THING, typename LANE>
ImVec2 TimelineCanvasMixin<THING, LANE>::screenToCanvas(const ImVec2& pos) {
    ImVec2 windowPos = ImGui::GetWindowPos();
    float scrollX = ImGui::GetScrollX();
    float scrollY = ImGui::GetScrollY();
    float x = (pos.x - windowPos.x - offsetLeft() + scrollX) / _zoomX;
    float y = (pos.y - windowPos.y - offsetTop() + scrollY - offsetStart()) / _zoomY;
    return ImVec2(x, y);
}

template<class THING, typename LANE>
inline ImVec2 TimelineCanvasMixin<THING, LANE>::canvasToScreen(const ImVec2& pos) {
    ImVec2 windowPos = ImGui::GetWindowPos();
    float scrollX = ImGui::GetScrollX();
    float scrollY = ImGui::GetScrollY();
    float x = pos.x * _zoomX + windowPos.x + offsetLeft() - scrollX;
    float y = pos.y * _zoomY + windowPos.y + offsetTop() - scrollY + offsetStart();
    return ImVec2(x, y);
}

template<class THING, typename LANE>
void TimelineCanvasMixin<THING, LANE>::handleShortcut() {
    if (defineShortcut(ImGuiMod_Ctrl | ImGuiKey_A)) {
        _state._selectedThings = { _allThings.begin(), _allThings.end() };
    }
}

template class TimelineCanvasMixin<Clip, Lane>;
template class TimelineCanvasMixin<Note, int16_t>;
