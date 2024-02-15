#include "PianoRollWindow.h"
#include <mutex>
#include <imgui.h>
#include "Clip.h"
#include "AudioEngine.h"
#include "Composer.h"
#include "Grid.h"
#include "GuiUtil.h"
#include "Midi.h"

constexpr float KEYBOARD_HEIGHT = 30.0f;
constexpr float TIMELINE_WIDTH = 20.0f;
constexpr float TIMELINE_START_OFFSET = 10.0f;
constexpr float KEY_WIDTH = 30.0f;

static int16_t allLanes[128];

PianoRollWindow::PianoRollWindow(Composer* composer) : TimelineCanvasMixin(composer) {
    _zoomX = 1.0f;
    _zoomY = 40.0f;
    _grid = gGrids[1].get();
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

void PianoRollWindow::edit(Clip* clip) {
    _clip = clip;
    _show = true;
    _scrollHereXKey = "C4";
    _state = State{};
}

void PianoRollWindow::handleDoubleClick(Note* thing) {
    _composer->_commandManager.executeCommand(new DeleteNoteCommand(_clip->_sequence.get(), thing));
}

Note* PianoRollWindow::handleDoubleClick(double time, int16_t* lane) {
    // TODO undo
    Note* note = new Note();
    note->_time = time;
    note->_key = *lane;
    _clip->_sequence->_notes.emplace_back(note);
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

void PianoRollWindow::handleClickTimeline(double time) {
    std::lock_guard<std::mutex> lock(_composer->_audioEngine->mtx);
    _composer->_playTime = time + _clip->_time;
}

Note* PianoRollWindow::copyThing(Note* other) {
    Note* note = new Note(*other);
    _clip->_sequence->_notes.emplace_back(note);
    return note;
}

void PianoRollWindow::deleteThing(Note* note) {
    auto& notes = _clip->_sequence->_notes;
    auto it = std::ranges::find_if(notes, [note](const auto& x) { return x.get() == note; });
    if (it != notes.end()) {
        notes.erase(it);
    }
}

void PianoRollWindow::prepareAllThings() {
    _allThings.clear();
    for (auto& note : _clip->_sequence->_notes) {
        _allThings.push_back(note.get());
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
    return _allLanes[key];
}

float PianoRollWindow::xFromThing(Note* thing) {
    return thing->_key * KEY_WIDTH;
}

float PianoRollWindow::getLaneWidth(Note* /*thing*/) {
    return KEY_WIDTH;
}

void PianoRollWindow::handleShortcut() {
}

void PianoRollWindow::renderPalyhead() {
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
    std::string name = _clip->_name;
    if (name.empty()) {
        name = "Piano Roll";
    }
    return name + "##Piano Roll";
}

std::string PianoRollWindow::canvasName() {
    return "##Piano Roll Canvas";
}
