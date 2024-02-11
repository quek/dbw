#include "TimelineCanvasMixin.h"
#include "Clip.h"
#include "Grid.h"
#include "GuiUtil.h"
#include "TrackLane.h"

template<class THING, typename LANE>
TimelineCanvasMixin<THING, LANE>::TimelineCanvasMixin(Composer* composer) : _composer(composer) {
}

template<class THING, typename LANE>
void TimelineCanvasMixin<THING, LANE>::handleMouse(ImVec2& clipRectMin, ImVec2& clipRectMax) {
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

    if (_state._draggingThing) {
        if (_state._thingClickedPart == Middle) {
            float time = timeFromMousePos(_state._thingClickedOffset);
            float timeDelta = time - _state._draggingThing->_time;
            // TODO TrackLane* lane = laneFromMousePos();
            // TODO laneDelta pointer?
            //int16_t key = noteKeyFromMouserPos();
            //int16_t keyDelta = key - _state._draggingthing->_key;
            //_state._draggingthing->_key = key;
            for (auto thing : _state._selectedThings) {
                thing->_time += timeDelta;
                // TODO lane note->_key += keyDelta;
            }
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
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        } else if (_state._thingClickedPart == Bottom) {
            float time = timeFromMousePos();
            float timeDelta = time - _state._draggingThing->_time - _state._draggingThing->_duration;
            if (std::ranges::all_of(_state._selectedThings,
                                    [timeDelta](auto x) { return x->_duration > -timeDelta; })) {
                for (auto thing : _state._selectedThings) {
                    thing->_duration += timeDelta;
                }
            }
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            // ノートのドラッグ解除
            _state._draggingThing = nullptr;
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
            _state._draggingThing = _state._clickedThing;
        } else {
            // 範囲選択
            _state._rangeSelecting = true;
            _state._selectedThingsAtStartRangeSelecting = _state._selectedThings;
        }
    } else if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        THING* thing = thingAtPos(mousePos);
        if (thing) {
            handleDoubleClick(thing);
        } else {
            float time = timeFromMousePos(_state._thingClickedOffset);
            LANE* lane = laneFromMousePos();
            handleDoubleClick(time, lane);
        }
    } else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        THING* thing = thingAtPos(mousePos);
        if (thing) {
            _state._unselectClickedThingIfMouserReleased = io.KeyCtrl && _state._selectedThings.contains(thing);
            if (!io.KeyCtrl && !_state._selectedThings.contains(thing)) {
                _state._selectedThings.clear();
            }
            _state._selectedThings.insert(thing);
            _state._clickedThing = thing;
            _state._consumedClicked = true;
            Bounds& bounds = _state._thingBoundsMap[thing];
            if (mousePos.y - bounds.p.y <= 5) {
                _state._thingClickedPart = Top;
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
            } else if (bounds.q.y - mousePos.y <= 5) {
                _state._thingClickedPart = Bottom;
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
            } else {
                _state._thingClickedPart = Middle;
                _state._thingClickedOffset = mousePos.y - bounds.p.y;
            }
        } else {
            _state._clickedThing = nullptr;
            if (!io.KeyCtrl && !io.KeyShift) {
                _state._selectedThings.clear();
            }
        }
    } else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        THING* thing = thingAtPos(mousePos);
        if (_state._unselectClickedThingIfMouserReleased) {
            if (thing == _state._clickedThing) {
                _state._selectedThings.erase(thing);
            }
            _state._unselectClickedThingIfMouserReleased = false;
        } else if (!io.KeyCtrl && _state._selectedThings.size() > 1 && thing == _state._clickedThing) {
            _state._selectedThings.clear();
            _state._selectedThings.insert(thing);
        }
    }
}

template<class THING, typename LANE>
void TimelineCanvasMixin<THING, LANE>::renderThing(ImVec2& windowPos) {
    _state._thingBoundsMap.clear();
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mousePos = io.MousePos;
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    float scrollX = ImGui::GetScrollX();
    float scrollY = ImGui::GetScrollY();
    for (auto& thing : _allThings) {
        float x1 = xFromThing(thing) + offsetLeft();
        float x2 = x1 + getLaneWidth(thing);
        float y1 = thing->_time * _zoomY - scrollY + offsetTop();
        float y2 = y1 + thing->_duration * _zoomY;
        ImVec2 pos1 = ImVec2(x1 - scrollX + 2.0f, y1) + windowPos;
        ImVec2 pos2 = ImVec2(x2 - scrollX - 1.0f, y2) + windowPos;
        ImU32 thingColor;
        if (_state._selectedThings.contains(thing)) {
            thingColor = colorSlectedThing();
        } else {
            thingColor = colorThing();
        }
        drawList->AddRectFilled(pos1, pos2, thingColor);
        _state._thingBoundsMap[thing] = Bounds(pos1, pos2);
    }
}

template<class THING, typename LANE>
double TimelineCanvasMixin<THING, LANE>::timeFromMousePos(float offset) {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mousePos = io.MousePos - ImVec2(offset, 0.0f);
    ImVec2 canvasPos = toCanvasPos(mousePos);
    double time = toSnapRound(canvasPos.y);
    return time;
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
ImVec2 TimelineCanvasMixin<THING, LANE>::toCanvasPos(ImVec2& pos) const {
    ImVec2 windowPos = ImGui::GetWindowPos();
    float scrollX = ImGui::GetScrollX();
    float scrollY = ImGui::GetScrollY();
    float x = (pos.x - windowPos.x - offsetLeft() + scrollX) / _zoomX;
    float y = (pos.y - windowPos.y - offsetTop() + scrollY - offsetStart()) / _zoomY;
    return ImVec2(x, y);
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

template class TimelineCanvasMixin<Clip, TrackLane>;
template class TimelineCanvasMixin<Note, uint16_t>;
