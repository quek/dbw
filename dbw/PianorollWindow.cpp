#include "PianoRollWindow.h"
#include <mutex>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "App.h"
#include "NoteClip.h"
#include "Composer.h"
#include "Grid.h"
#include "GuiUtil.h"
#include "Midi.h"
#include "command/AddNotes.h"
#include "command/DeleteNotes.h"
#include "command/DuplicateNotes.h"

constexpr float KEYBOARD_HEIGHT = 30.0f;
constexpr float TIMELINE_WIDTH = 20.0f;
constexpr float TIMELINE_START_OFFSET = 10.0f;
constexpr float KEY_WIDTH = 30.0f;

static int16_t allLanes[128];

PianoRollWindow::PianoRollWindow(Composer* composer) : TimelineCanvasMixin(composer) {
    _zoomX = 1.0f;
    _zoomY = 40.0f;
    _grid = gGrids[2].get();
    for (int16_t i = 0; i < 128; ++i) {
        allLanes[i] = i;
        _allLanes.push_back(&allLanes[i]);
    }
}

void PianoRollWindow::render() {
    if (!_show) {
        return;
    }
    TimelineCanvasMixin::render();
}

void PianoRollWindow::edit(NoteClip* clip) {
    _clip = clip;
    _show = true;
    _scrollHereXKey = "C4";
    _state = State{};
    if (!clip->_sequence->getItems().empty()) {
        _state._defaultThingDuration = clip->_sequence->getItems()[0]->_duration;
    } else {
        _state._defaultThingDuration = _grid->_unit;
    }
}

void PianoRollWindow::handleDoubleClick(Note* /*thing*/) {
    // TODO
    //_composer->_commandManager.executeCommand(new DeleteNoteCommand(_clip->_sequence.get(), thing));
}

Note* PianoRollWindow::handleDoubleClick(double time, int16_t* lane) {
    Note* note = new Note();
    note->_time = time;
    note->_key = *lane;
    note->_duration = _state._defaultThingDuration;
    std::set<Note*> notes({ note });
    _composer->_commandManager.executeCommand(new command::AddNotes(_clip->_sequence.get(), notes, true));
    return note;
}

void PianoRollWindow::handleMove(double oldTime, double newTime, int16_t* oldLane, int16_t* newLane) {
    double timeDelta = newTime - oldTime;
    double keyDelta = *newLane - *oldLane;
    for (auto& note : _state._draggingThings) {
        note->_time += timeDelta;
        note->_key += keyDelta;
    }
}

void PianoRollWindow::handleClickTimeline(double time, bool ctrl, bool alt) {
    std::lock_guard<std::recursive_mutex> lock(_composer->app()->_mtx);
    double value = time + _clip->_time;
    if (ctrl) {
        _composer->_loopStartTime = value;
    } else if (alt) {
        _composer->_loopEndTime = value;
    } else {
        _composer->_playTime = value;
    }
    if (_composer->_loopStartTime > _composer->_loopEndTime) {
        std::swap(_composer->_loopStartTime, _composer->_loopEndTime);
    }
}

std::pair<std::set<Note*>, Command*> PianoRollWindow::copyThings(std::set<Note*> srcs, bool redoable) {
    std::set<Note*> notes;
    for (auto& it : srcs) {
        Note* note = new Note(*it);
        notes.insert(note);
    }
    return { notes, new command::AddNotes(_clip->_sequence.get(), notes, redoable) };
}

Command* PianoRollWindow::deleteThings(std::set<Note*>& notes, bool undoable) {
    return new command::DeleteNotes(_clip->_sequence.get(), notes, undoable);
}

Command* PianoRollWindow::duplicateThings(std::set<Note*>& notes, bool undoable) {
    return new command::DuplicateNotes(_clip->_sequence.get(), notes, undoable);
}

void PianoRollWindow::prepareAllThings() {
    _allThings.clear();
    for (auto& note : _clip->_sequence->getItems()) {
        _allThings.push_back((Note*)note.get());
    }
}

float PianoRollWindow::offsetTop() const {
    return KEYBOARD_HEIGHT;
}

float PianoRollWindow::offsetLeft() const {
    return TIMELINE_WIDTH;
}

float PianoRollWindow::offsetStart() const {
    return TIMELINE_START_OFFSET;
}

ImU32 PianoRollWindow::colorSlectedThing() {
    return SELECTED_NOTE_COLOR;
}

ImU32 PianoRollWindow::colorThing() {
    return NOTE_COLOR;
}

int16_t* PianoRollWindow::laneFromPos(ImVec2& pos) {
    ImVec2 windowPos = ImGui::GetWindowPos();
    float scrollX = ImGui::GetScrollX();
    float x = (pos.x - windowPos.x - offsetLeft() + scrollX);
    x = x / KEY_WIDTH / _zoomX;
    int16_t key = static_cast<int16_t>(x);
    if (key < 0) {
        key = 0;
    } else if (key > 127) {
        key = 127;
    }
    return _allLanes[key];
}

float PianoRollWindow::xFromThing(Note* thing) {
    return laneToScreenX(_allLanes[thing->_key]);
}

float PianoRollWindow::laneToScreenX(int16_t* lane) {
    return KEY_WIDTH * *lane * _zoomX + offsetLeft() - ImGui::GetScrollX() + ImGui::GetWindowPos().x;
}

float PianoRollWindow::getLaneWidth(Note* /*thing*/) {
    return KEY_WIDTH;
}

void PianoRollWindow::handleShortcut() {
    if (defineShortcut(ImGuiKey_Escape)) {
        _show = false;
    }

    TimelineCanvasMixin::handleShortcut();
}

void PianoRollWindow::renderPlayhead() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    float scrollX = ImGui::GetScrollX();
    ImVec2 pos1 = canvasToScreen(ImVec2(scrollX, fmod(_composer->_playTime, _clip->_sequence->_duration)));
    ImVec2 pos2 = pos1 + ImVec2(ImGui::GetWindowWidth(), 0.0f);
    drawList->AddLine(pos1, pos2, PLAY_CURSOR_COLOR);
}

void PianoRollWindow::renderHeader() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImGui::BeginGroup();
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    float scrollX = ImGui::GetScrollX();
    float scrollY = ImGui::GetScrollY();
    for (int i = 0; i <= 127; ++i) {
        auto& note = gMidiNumToSym[i];
        auto mod = i % 12;
        ImU32 backgroundColor = BACKGROUD_WHITE_KEY_COLOR;
        if (mod == 1 || mod == 3 || mod == 6 || mod == 8 || mod == 10) {
            backgroundColor = BACKGROUD_BLACK_KEY_COLOR;
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32_BLACK);
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_WHITE);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32_WHITE);
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_BLACK);
        }
        float x = KEY_WIDTH * i * _zoomX + offsetLeft();
        float y = scrollY;
        ImGui::SetCursorPos(ImVec2(x, y));
        if (ImGui::Button(note.c_str(), ImVec2(KEY_WIDTH * _zoomX, offsetTop()))) {
            // TODO 音とか鳴らしたいよね
        }
        if (note == _scrollHereXKey) {
            ImGui::SetScrollHereX();
            _scrollHereXKey = "";
        }
        ImGui::PopStyleColor(2);

        float x1 = x - scrollX;
        float x2 = x1 + KEY_WIDTH * _zoomX;
        float y1 = offsetTop();
        float y2 = y1 + ImGui::GetWindowHeight();
        drawList->AddRectFilled(ImVec2(x1, y1) + windowPos, ImVec2(x2, y2) + windowPos, backgroundColor);

        drawList->AddLine(ImVec2(x1, y1) + windowPos, ImVec2(x1, y2) + windowPos, BEAT_LINE_COLOR);
    }
    ImGui::PopStyleVar(2);
    ImGui::EndGroup();
}

std::string PianoRollWindow::windowName() {
    return "Piano Roll";
    //std::string name = _clip->_name;
    //if (name.empty()) {
    //    name = "Piano Roll";
    //}
    //return name + "##Piano Roll";
}

std::string PianoRollWindow::canvasName() {
    return "##Piano Roll Canvas";
}
