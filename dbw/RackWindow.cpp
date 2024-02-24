#include "RackWindow.h"
#include <ranges>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "Composer.h"
#include "Track.h"

RackWindow::RackWindow(Composer* composer) : _composer(composer) {
}

void RackWindow::render() {
    if (ImGui::Begin("Rack")) {

        if (ImGui::BeginChild("Rack Canvas",
                              ImVec2(0.0f, 0.0f),
                              ImGuiChildFlags_None,
                              ImGuiWindowFlags_HorizontalScrollbar)) {
            for (auto& track : _composer->allTracks()) {
                ImGui::Text(track->_name.c_str());
            }
            ImGui::EndChild();
        }
    }
    ImGui::End();
}
