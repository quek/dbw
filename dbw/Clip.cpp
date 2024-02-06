#include "Clip.h"
#include <imgui.h>
#include "PianoRoll.h"

Clip::Clip() {
    _sequence = std::make_shared<Sequence>();
}

Clip::Clip(std::shared_ptr<Sequence> sequence) : _sequence(sequence) {
}

void Clip::renderInScene() {
    ImGui::PushID(this);
    if (ImGui::Selectable("clip", false, ImGuiSelectableFlags_AllowDoubleClick)) {
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            gPianoRoll->edit(this);
        }
    }
    ImGui::PopID();
}
