#include "Line.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <algorithm>

void Line::render()
{
    ImGui::PushID(this);
    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 3);
    if (ImGui::InputText("##note", &_note)) {
        std::transform(_note.begin(), _note.end(), _note.begin(),
            [](auto c) { return static_cast<char>(std::toupper(c)); });
    }
    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 2);
    ImGui::SameLine();
    if (ImGui::DragInt("##velocity", &_velocity, 1.0f, 0, 0x7f, "%02X")) {
    }
    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 2);
    ImGui::SameLine();
    if (ImGui::DragInt("##delay", &_delay, 1.0f, 0, 0xff, "%02X")) {
    }
    ImGui::PopID();
}
