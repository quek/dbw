#include "Clip.h"
#include <imgui.h>
#include "PianoRollH.h"

Clip::Clip(double time, double duration) :
    Thing(time, duration), _sequence(std::make_shared<Sequence>(duration)) {
}

Clip::Clip(std::shared_ptr<Sequence> sequence) : Thing(0.0f, sequence->_duration), _sequence(sequence) {
}

void Clip::renderInScene(PianoRollH* pianoRoll) {
    ImGui::PushID(this);
    if (ImGui::Selectable("clip", false, ImGuiSelectableFlags_AllowDoubleClick)) {
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            pianoRoll->edit(this);
        }
    }
    ImGui::PopID();
}
