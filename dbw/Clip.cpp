#include "Clip.h"
#include <imgui.h>

void Clip::renderInScene() {
    ImGui::PushID(this);
    if (ImGui::Button("▶")) {
        play();
    }
    ImGui::SameLine();
    ImGui::Text("clip");
    ImGui::PopID();
}

void Clip::play() {
    // TODO
}
