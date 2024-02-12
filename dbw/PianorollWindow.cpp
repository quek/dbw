#include "PianoRollWindow.h"
#include <imgui.h>
#include "Grid.h"
#include "GuiUtil.h"

constexpr float KEYBOARD_HEIGHT = 30.0f;
constexpr float TIMELINE_WIDTH = 15.0f;
constexpr float TIMELINE_START_OFFSET = 10.0f;

PianoRollWindow::PianoRollWindow(Composer* composer) : TimelineCanvasMixin(composer) {
    _zoomX = 1.0f;
    _zoomY = 10.0f;
    _grid = gGrids[1].get();
}

void PianoRollWindow::render() {
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
                ImVec2 clipRectMin = windowPos + ImVec2(offsetLeft(), offsetTop());
                ImVec2 clipRectMax = ImVec2(clipRectMin.x + ImGui::GetWindowWidth(), clipRectMin.y + ImGui::GetWindowHeight());
                ImGui::PushClipRect(clipRectMin, clipRectMax, true);
                {
                    //renderGrid();
                    //renderBackgroud();
                    //renderNotes();
                    //handleCanvas();
                }
                ImGui::PopClipRect();
                renderTimeline();
                //renderKeyboard();
            }
            ImGui::EndChild();
        }
        ImGui::PopStyleVar();

        // TODO マウスホイールとかでスクロールするようにする
        renderDebugZoomSlider();
    }
    ImGui::End();
}

void PianoRollWindow::edit(Clip* clip) {
    _clip = clip;
    _show = true;
    _scrollHereYKey = "C4";
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
