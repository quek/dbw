#include "PianoRollWindow.h"
#include <imgui.h>
#include "Clip.h"
#include "Grid.h"
#include "GuiUtil.h"
#include "Midi.h"

constexpr float KEYBOARD_HEIGHT = 30.0f;
constexpr float TIMELINE_WIDTH = 20.0f;
constexpr float TIMELINE_START_OFFSET = 10.0f;
constexpr float KEY_WIDTH = 30.0f;

PianoRollWindow::PianoRollWindow(Composer* composer) : TimelineCanvasMixin(composer) {
    _zoomX = 1.0f;
    _zoomY = 10.0f;
    _grid = gGrids[1].get();
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

void PianoRollWindow::handleShortcut() {
}

void PianoRollWindow::renderPalyCursor() {
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
