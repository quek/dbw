#include "Clip.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "PianoRollWindow.h"

Clip::Clip(double time, double duration) :
    Thing(time, duration), _sequence(Sequence::create(duration)) {
}

Clip::Clip(double time, double duration, std::shared_ptr<Sequence> sequence) :
    Thing(time, duration), _sequence(sequence) {
}

Clip::Clip(std::shared_ptr<Sequence> sequence) : Thing(0.0f, sequence->_duration), _sequence(sequence) {
}

std::string Clip::name() const {
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

tinyxml2::XMLElement* Clip::toXml(tinyxml2::XMLDocument* doc) {
    auto element = doc->NewElement("Clip");
    element->SetAttribute("id", xmlId());
    element->SetAttribute("time", _time);
    element->SetAttribute("duration", _duration);
    element->InsertEndChild(_sequence->toXml(doc));
    return element;
}

std::unique_ptr<Clip> Clip::fromXml(tinyxml2::XMLElement* element) {
    double time = 0.0, duration = 16.0;
    element->QueryDoubleAttribute("time", &time);
    element->QueryDoubleAttribute("duration", &duration);
    std::shared_ptr<Sequence> sequence = Sequence::fromXml(element->FirstChildElement("Notes"));
    std::unique_ptr<Clip> clip(new Clip(time, duration, sequence));
    return clip;
}
