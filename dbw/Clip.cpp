#include "Clip.h"
#include <imgui.h>
#include "PianoRoll.h"

void Clip::renderInScene() {
    ImGui::PushID(this);
    if (ImGui::Selectable("clip", false, ImGuiSelectableFlags_AllowDoubleClick)) {
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            gPianoRoll->edit(this);
        }
    }
    ImGui::PopID();
}
