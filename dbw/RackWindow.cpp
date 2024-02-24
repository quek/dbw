#include "RackWindow.h"
#include <ranges>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "Composer.h"
#include "Fader.h"
#include "Track.h"

RackWindow::RackWindow(Composer* composer) : _composer(composer) {
}

void RackWindow::render() {
    if (ImGui::Begin("Rack")) {

        if (ImGui::BeginChild("Rack Canvas",
                              ImVec2(0.0f, 0.0f),
                              ImGuiChildFlags_None,
                              ImGuiWindowFlags_HorizontalScrollbar)) {
            renderHeader();
            ImGui::EndChild();
        }
    }
    ImGui::End();
}

void RackWindow::renderHeader() {
    for (auto& track : _composer->allTracks()) {
        ImGui::BeginGroup();
        ImGui::Selectable(track->_name.c_str(), &track->_selected, ImGuiSelectableFlags_None, ImVec2(track->_width, TEXT_BASE_HEIGHT));
        track->_fader->render(track->_width);
        ImGui::EndGroup();
        ImGui::SameLine();
    }
}

