#include "Clip.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "PianoRollWindow.h"

Clip::Clip(double time, double duration) :
    Thing(time, duration), _sequence(std::make_shared<Sequence>(duration)) {
}

Clip::Clip(std::shared_ptr<Sequence> sequence) : Thing(0.0f, sequence->_duration), _sequence(sequence) {
}

std::string Clip::name() {
    std::string name = (_sequence.use_count() > 1 ? "∞" : "") + _sequence->_name;
    return name;
}

void Clip::renderInScene(PianoRollWindow* pianoRoll) {
    std::string sequenceName = name();

    ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(-4.0f, 1.0f);
    ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeightWithSpacing()) + ImVec2(8.0f, 0.0f);
    ImGui::GetWindowDrawList()->AddRectFilled(pos, pos + size, IM_COL32(120, 120, 120, 255)); // 背景色を描画
    ImGui::PushID(this);
    if (ImGui::Selectable(sequenceName.c_str(), &_selected, ImGuiSelectableFlags_AllowDoubleClick)) {
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            pianoRoll->edit(this);
        }
    }
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
        ImGui::SetDragDropPayload("Sequence Matrix Clip", this, sizeof(*this));
        ImGui::Text(sequenceName.c_str());
        ImGui::EndDragDropSource();
    }
    ImGui::PopID();
}
