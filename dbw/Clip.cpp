#include "Clip.h"
#include <imgui.h>
#include "PianoRoll.h"

Clip::Clip(double time, double duration) :
    _time(time), _duration(duration), _sequence(std::make_shared<Sequence>(duration)) {
}

Clip::Clip(std::shared_ptr<Sequence> sequence) : _time(0), _duration(sequence->_duration), _sequence(sequence) {
}

void Clip::renderInScene(PianoRoll* pianoRoll) {
    ImGui::PushID(this);
    if (ImGui::Selectable("clip", false, ImGuiSelectableFlags_AllowDoubleClick)) {
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            pianoRoll->edit(this);
        }
    }
    ImGui::PopID();
}
