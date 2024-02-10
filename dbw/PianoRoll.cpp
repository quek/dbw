#include "PianoRoll.h"
#include <memory>
#include "Clip.h"
#include "Composer.h"
#include "Grid.h"
#include "GuiUtil.h"
#include "Midi.h"
#include "Note.h"

constexpr float BEAT_WIDTH = 10.0f;
constexpr float KEYBOARD_WIDTH = 30.0f;
constexpr float TIMELINE_HEIGHT = 15.0f;
constexpr float TIMELINE_START_OFFSET = 10.0f;
constexpr float GRID_SKIP_WIDTH = 20.0f;
float KEY_HEIGHT = 50.0f;

ImU32 BACKGROUD_WHITE_KEY_COLOR = IM_COL32(0x22, 0x22, 0x22, 0x88);
ImU32 BACKGROUD_BLACK_KEY_COLOR = IM_COL32(0x00, 0x00, 0x00, 0x88);
ImU32 NOTE_COLOR = IM_COL32(0x00, 0xcc, 0xcc, 0x88);
ImU32 SELECTED_NOTE_COLOR = IM_COL32(0x66, 0x66, 0xff, 0x88);

PianoRoll::PianoRoll(Composer* composer) : ZoomMixin(4.0f, 0.5f), _composer(composer) {
    _grid = gGrids[1].get();
}

void PianoRoll::render() {
    if (!_show) return;

    _state.reset();

    if (ImGui::Begin("Piano Roll", &_show)) {
        renderGridSnap();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        {
            if (ImGui::BeginChild("##Piano Roll Canvas",
                                  ImVec2(0.0f, -22.0f),
                                  ImGuiChildFlags_None,
                                  ImGuiWindowFlags_HorizontalScrollbar)) {
                ImVec2 windowPos = ImGui::GetWindowPos();
                ImVec2 clipRectMin = windowPos + ImVec2(KEYBOARD_WIDTH, TIMELINE_HEIGHT);
                ImVec2 clipRectMax = ImVec2(clipRectMin.x + ImGui::GetWindowWidth(), clipRectMin.y + ImGui::GetWindowHeight());
                ImGui::PushClipRect(clipRectMin, clipRectMax, true);
                {
                    renderGrid();
                    renderBackgroud();
                    renderNotes();
                    handleCanvas();
                }
                ImGui::PopClipRect();
                renderTimeline();
                renderKeyboard();
            }
            ImGui::EndChild();
        }
        ImGui::PopStyleVar();

        // TODO マウスホイールとかでスクロールするようにする
        renderDebugZoomSlider();
    }
    ImGui::End();
}

void PianoRoll::edit(Clip* clip) {
    _clip = clip;
    _show = true;
    _scrollHereYKey = "C4";
    _state = State{};
}

int PianoRoll::maxBar() {
    return _composer->maxBar();
}

void PianoRoll::renderBackgroud() const {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();
    float scrollY = ImGui::GetScrollY();
    float windowWidth = ImGui::GetWindowWidth();
    float x1 = windowPos.x + KEYBOARD_WIDTH;
    float x2 = x1 + windowWidth + KEYBOARD_WIDTH;
    for (int i = 127; i >= 0; --i) {
        auto mod = i % 12;
        ImU32 color = BACKGROUD_WHITE_KEY_COLOR;
        if (mod == 1 || mod == 3 || mod == 6 || mod == 8 || mod == 10) {
            color = BACKGROUD_BLACK_KEY_COLOR;
        }
        float y1 = KEY_HEIGHT * i * _zoomY + windowPos.y - scrollY + TIMELINE_HEIGHT;
        float y2 = KEY_HEIGHT * (i + 1) * _zoomY + windowPos.y - scrollY + TIMELINE_HEIGHT;
        drawList->AddRectFilled(ImVec2(x1, y1), ImVec2(x2, y2), color);
    }
}

void PianoRoll::renderGrid() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();
    float windowHeight = ImGui::GetWindowHeight();
    float y1 = windowPos.y;
    float y2 = windowPos.y + windowHeight;
    float lastBarX = -GRID_SKIP_WIDTH;
    float scrollX = ImGui::GetScrollX();
    for (int bar = 0; bar < maxBar(); ++bar) {
        float barX = (BEAT_WIDTH * 4 * bar * _zoomX) + windowPos.x + TIMELINE_START_OFFSET + KEYBOARD_WIDTH - scrollX;
        if (barX - lastBarX < GRID_SKIP_WIDTH) {
            continue;
        }
        drawList->AddLine(ImVec2(barX, y1), ImVec2(barX, y2), BAR_LINE_COLOR);

        renderGridBeat16th(drawList, barX, y1, y2);

        if (BEAT_WIDTH * _zoomX >= GRID_SKIP_WIDTH) {
            for (int beat = 1; beat < 4; ++beat) {
                float beatX = barX + (BEAT_WIDTH * beat * _zoomX);
                drawList->AddLine(ImVec2(beatX, y1), ImVec2(beatX, y2), BEAT_LINE_COLOR);

                renderGridBeat16th(drawList, beatX, y1, y2);
            }
        } else if (BEAT_WIDTH * 2 * _zoomX >= GRID_SKIP_WIDTH) {
            float beatX = barX + (BEAT_WIDTH * 2 * _zoomX);
            drawList->AddLine(ImVec2(beatX, y1), ImVec2(beatX, y2), BEAT_LINE_COLOR);
        }

        lastBarX = barX;
    }
}

void PianoRoll::renderGridBeat16th(ImDrawList* drawList, float beatX, float y1, float y2) const {
    if (BEAT_WIDTH / 4 * _zoomX >= GRID_SKIP_WIDTH) {
        for (int beat16th = 1; beat16th < 4; ++beat16th) {
            float beat16thX = beatX + (BEAT_WIDTH / 4 * beat16th * _zoomX);
            drawList->AddLine(ImVec2(beat16thX, y1), ImVec2(beat16thX, y2), BEAT16TH_LINE_COLOR, 0.5f);
        }
    }
}

void PianoRoll::renderKeyboard() {
    ImGui::BeginGroup();
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    float scrollX = ImGui::GetScrollX();
    for (int i = 127; i >= 0; --i) {
        auto& note = gMidiNumToSym[i];
        auto mod = i % 12;
        if (mod == 1 || mod == 3 || mod == 6 || mod == 8 || mod == 10) {
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32_BLACK);
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_WHITE);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32_WHITE);
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_BLACK);
        }
        ImGui::SetCursorPos(ImVec2(scrollX, KEY_HEIGHT * (127 - i) * _zoomY + TIMELINE_HEIGHT));
        if (ImGui::Button(note.c_str(), ImVec2(KEYBOARD_WIDTH, KEY_HEIGHT * _zoomY))) {
            // ...
        }
        if (note == _scrollHereYKey) {
            ImGui::SetScrollHereY();
            _scrollHereYKey = "";
        }
        ImGui::PopStyleColor(2);
    }
    ImGui::PopStyleVar(2);
    ImGui::EndGroup();
}

void PianoRoll::renderNotes() {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2& mousePos = io.MousePos;
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();
    float scrollX = ImGui::GetScrollX();
    float scrollY = ImGui::GetScrollY();
    ImVec2 scrollPos(scrollX, scrollY);
    for (auto& note : _clip->_sequence->_notes) {
        ImGui::PushID(note.get());
        float x1 = static_cast<float>(note->_time * BEAT_WIDTH * _zoomX + KEYBOARD_WIDTH + TIMELINE_START_OFFSET);
        float y1 = (127 - note->_key) * KEY_HEIGHT * _zoomY + TIMELINE_HEIGHT;
        float x2 = static_cast<float>(x1 + note->_duration * BEAT_WIDTH * _zoomX);
        float y2 = y1 + KEY_HEIGHT * _zoomY;
        ImVec2 pos1 = ImVec2(x1, y1) + windowPos - scrollPos + ImVec2(0.0f, 1.0f);
        ImVec2 pos2 = ImVec2(x2, y2) + windowPos - scrollPos + ImVec2(0.0f, -1.0f);
        ImU32 noteColor;
        if (_state._selectedNotes.contains(note.get())) {
            noteColor = SELECTED_NOTE_COLOR;
        } else {
            noteColor = NOTE_COLOR;
        }
        drawList->AddRectFilled(pos1, pos2,
                                noteColor,
                                2.0f, ImDrawFlags_RoundCornersAll);
        auto label = numberToNote(note->_key);
        drawList->AddText(pos1 + ImVec2(2.0f, 0.0f), IM_COL32_WHITE, label.c_str());

        Bounds noteBounds(pos1, pos2);
        if (noteBounds.contains(mousePos)) {
            if (!_state._draggingNote) {
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    _state._unselectClickedNoteIfMouserReleased = io.KeyCtrl && _state._selectedNotes.contains(note.get());
                    if (!io.KeyCtrl && !_state._selectedNotes.contains(note.get())) {
                        _state._selectedNotes.clear();
                    }
                    _state._selectedNotes.insert(note.get());
                    _state._clickedNote = note.get();
                    _state._consumedClicked = true;
                } else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                    if (_state._unselectClickedNoteIfMouserReleased) {
                        _state._selectedNotes.erase(_state._clickedNote);
                    }
                    _state._unselectClickedNoteIfMouserReleased = false;
                }
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    _composer->_commandManager.executeCommand(new DeleteNoteCommand(_clip->_sequence.get(), note.get()));
                    _state._consumedDoubleClick = true;
                }
                if (mousePos.x - pos1.x <= 5) {
                    _state._noteClickedPart = Left;
                    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                } else if (pos2.x - mousePos.x <= 5) {
                    _state._noteClickedPart = Right;
                    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                } else {
                    _state._noteClickedPart = Middle;
                    _state._noteClickedOffset = mousePos.x - pos1.x;
                }
            }
        }

        ImGui::PopID();
    }
}

void PianoRoll::handleCanvas() {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2& mousePos = io.MousePos;
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();
    float scrollX = ImGui::GetScrollX();
    float scrollY = ImGui::GetScrollY();

    {   // For Debug
        ImGui::SetCursorPos(ImVec2(KEYBOARD_WIDTH + scrollX + 10.0f, 100 + scrollY));
        ImGui::BeginGroup();
        ImGui::Text("Mouse %f %f", mousePos.x, mousePos.y);
        ImGui::Text("GetWindowPos%f %f", windowPos.x, windowPos.y);
        ImGui::Text("GetWindowSize %f %f", windowSize.x, windowSize.y);
        ImGui::Text("GetScrollX/Y %f %f", scrollX, scrollY);
        ImGui::Text("Notes: %d", _clip->_sequence->_notes.size());
        ImVec2 canvasPos = toCanvasPos(mousePos);
        float time = canvasPos.x / BEAT_WIDTH;
        int16_t key = static_cast<int16_t>(128 - canvasPos.y / KEY_HEIGHT);
        ImGui::Text("time %f key %d %s", time, key, numberToNote(key).c_str());
        ImGui::Text("_noteClickedOffset %f", _state._noteClickedOffset);
    }

    if (isInCanvas(mousePos)) {
        if (!_state._consumedDoubleClick && io.MouseDoubleClicked[0]) {
            Note* note = noteFromMousePos();
            if (note) {
                _clip->_sequence->_notes.emplace_back(note);
                if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                    _state._clickedNote = note;
                    _state._selectedNotes.insert(note);
                    _state._draggingNote = note;
                    _state._noteClickedPart = Right;
                }
            }
        }

        if (_state._draggingNote) {
            if (_state._noteClickedPart == Middle) {
                float time = noteTimeFromMouserPos(_state._noteClickedOffset);
                float timeDelta = time - _state._draggingNote->_time;
                int16_t key = noteKeyFromMouserPos();
                int16_t keyDelta = key - _state._draggingNote->_key;
                _state._draggingNote->_time = time;
                _state._draggingNote->_key = key;
                ImGui::Text("draggingNote %p time %f key %s", _state._draggingNote, time, numberToNote(key).c_str());
                for (auto note : _state._selectedNotes) {
                    if (note != _state._draggingNote) {
                        note->_time += timeDelta;
                        note->_key += keyDelta;
                    }
                }
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
            } else if (_state._noteClickedPart == Left) {
                float time = noteTimeFromMouserPos();
                float timeDelta = time - _state._draggingNote->_time;
                if (std::ranges::all_of(_state._selectedNotes,
                                        [timeDelta](auto x) { return x->_duration > timeDelta; })) {
                    for (auto note : _state._selectedNotes) {
                        note->_time += timeDelta;
                        note->_duration += -timeDelta;
                    }
                }
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            } else if (_state._noteClickedPart == Right) {
                float time = noteTimeFromMouserPos();
                float timeDelta = time - _state._draggingNote->_time - _state._draggingNote->_duration;
                if (std::ranges::all_of(_state._selectedNotes,
                                        [timeDelta](auto x) { return x->_duration > -timeDelta; })) {
                    for (auto note : _state._selectedNotes) {
                        note->_duration += timeDelta;
                    }
                }
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            }
            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                // ノートのドラッグ解除
                _state._draggingNote = nullptr;
            }
        } else if (_state._rangeSelecting) {
            // Ctrl でトグル、Shift で追加 Reason の仕様がいいと思う
            ImVec2 pos1 = io.MouseClickedPos[ImGuiMouseButton_Left];
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            drawList->AddRect(pos1, mousePos, RANGE_SELECTING_COLOR, 0.0f, ImDrawFlags_None, 3.0f);
            if (!io.KeyShift && !io.KeyCtrl) {
                _state._selectedNotes.clear();
            }
            Bounds bounds(pos1, mousePos);
            for (auto& note : _clip->_sequence->_notes) {
                Bounds noteBounds = boundsOfNote(note.get());
                if (bounds.overlaped(noteBounds)) {
                    if (io.KeyCtrl) {
                        if (_state._selectedNotes.contains(note.get())) {
                            _state._selectedNotes.erase(note.get());
                        } else {
                            _state._selectedNotes.insert(note.get());
                        }
                    } else {
                        _state._selectedNotes.insert(note.get());
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
                if (!_state._selectedNotes.empty() && _state._clickedNote) {
                    // ノートの移動 or 長さ変更
                    _state._draggingNote = _state._clickedNote;
                } else {
                    // 範囲選択
                    _state._rangeSelecting = true;
                }
            } else if (!_state._consumedClicked) {
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !io.KeyCtrl && !io.KeyShift) {
                    _state._selectedNotes.clear();
                }
                if (!ImGui::IsAnyMouseDown()) {
                    _state._clickedNote = nullptr;
                }
            }
        }
    }
    {   // FOR Debug
        ImGui::Text("draggingNote %p", _state._clickedNote);
        ImGui::Text("draggingNote %p", _state._draggingNote);
        ImGui::EndGroup();
    }
}

Bounds PianoRoll::boundsOfNote(Note* note) {
    ImVec2 windowPos = ImGui::GetWindowPos();
    float scrollX = ImGui::GetScrollX();
    float scrollY = ImGui::GetScrollY();
    ImVec2 scrollPos(scrollX, scrollY);
    float x1 = static_cast<float>(note->_time * BEAT_WIDTH * _zoomX + KEYBOARD_WIDTH + TIMELINE_START_OFFSET);
    float y1 = (127 - note->_key) * KEY_HEIGHT * _zoomY + TIMELINE_HEIGHT;
    float x2 = static_cast<float>(x1 + note->_duration * BEAT_WIDTH * _zoomX);
    float y2 = y1 + KEY_HEIGHT * _zoomY;
    ImVec2 pos1 = ImVec2(x1, y1) + windowPos - scrollPos + ImVec2(0.0f, 1.0f);
    ImVec2 pos2 = ImVec2(x2, y2) + windowPos - scrollPos + ImVec2(0.0f, -1.0f);
    return Bounds(pos1, pos2);
}

void PianoRoll::renderTimeline() {
    float leftPadding = 2.0f;
    float scrollY = ImGui::GetScrollY();
    float lastX = -GRID_SKIP_WIDTH;
    for (int i = 0; i < maxBar(); ++i) {
        float x = (BEAT_WIDTH * 4 * i * _zoomX) + KEYBOARD_WIDTH + TIMELINE_START_OFFSET;
        if (x - lastX < GRID_SKIP_WIDTH) {
            continue;
        } else {
            lastX = x;
        }
        float y = scrollY;
        ImVec2 pos = ImVec2(x + leftPadding, y);
        ImGui::SetCursorPos(pos);
        ImGui::Text(std::to_string(i + 1).c_str());
    }
}

Note* PianoRoll::noteFromMousePos() {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2& mousePos = io.MousePos;
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 canvasPos = toCanvasPos(mousePos);
    // FL の動きは toSnapRound だけど、それ以外はこっち。升目を埋めるイメージ
    float time = toSnapFloor(canvasPos.x / BEAT_WIDTH);
    int16_t key = static_cast<int16_t>(128 - canvasPos.y / KEY_HEIGHT);
    if (0 <= key && key <= 127) {
        Note* note = new Note(time, 1, key, 0.8f);
        return note;
    } else {
        return nullptr;
    }
}

double PianoRoll::noteTimeFromMouserPos(float offset) {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mousePos = io.MousePos - ImVec2(offset, 0.0f);
    ImVec2 canvasPos = toCanvasPos(mousePos);
    double time = toSnapRound(canvasPos.x / BEAT_WIDTH);
    return time;
}

int16_t PianoRoll::noteKeyFromMouserPos() {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2& mousePos = io.MousePos;
    ImVec2 canvasPos = toCanvasPos(mousePos);
    int16_t key = static_cast<int16_t>(128 - canvasPos.y / KEY_HEIGHT);
    return key;
}

ImVec2 PianoRoll::toCanvasPos(ImVec2& pos) const {
    ImVec2 windowPos = ImGui::GetWindowPos();
    float scrollX = ImGui::GetScrollX();
    float scrollY = ImGui::GetScrollY();
    float x = (pos.x - windowPos.x - KEYBOARD_WIDTH - TIMELINE_START_OFFSET + scrollX) / _zoomX;
    float y = (pos.y - windowPos.y - TIMELINE_HEIGHT + scrollY) / _zoomY;
    return ImVec2(x, y);
}

double PianoRoll::toSnapFloor(const double time) {
    if (!_snap) {
        return time;
    }
    return _grid->snapFloor(time);
}

double PianoRoll::toSnapRound(const double time) {
    if (!_snap) {
        return time;
    }
    return _grid->snapRound(time);
}

bool PianoRoll::isInCanvas(ImVec2& pos) {
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();
    return  (windowPos <= pos && pos < windowPos + windowSize);
}

void PianoRoll::State::reset() {
    _consumedDoubleClick = false;
    _consumedClicked = false;
}
