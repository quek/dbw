#include "Fader.h"
#include <ranges>
#include <imgui.h>

Fader::Fader(std::string name, Track* track) :
        BuiltinModule(name, track), _level(1.0) {
}

tinyxml2::XMLElement* Fader::dawProject(tinyxml2::XMLDocument* doc) {
    auto* element = doc->NewElement("BuiltinDevice");
    element->SetAttribute("deviceRole", "audioFX");
    element->SetAttribute("deviceName", _name.c_str());
    element->SetAttribute("deviceID", "Fader");
    auto* parameters = element->InsertNewChildElement("Parameters");
    auto* realParameter = parameters->InsertNewChildElement("RealParameter");
    realParameter->SetAttribute("name", "Level");
    realParameter->SetAttribute("unit", "linear");
    realParameter->SetAttribute("value", _level);

    return element;
}

void Fader::loadParameters(tinyxml2::XMLElement* element) {
    for (auto param = element->FirstChildElement();
         param != nullptr;
         param = param->NextSiblingElement()) {
        auto name = param->Attribute("name");
        if (strcmp(name, "level") == 0) {
            param->QueryFloatAttribute("value", &_level);
        }
    }

}

bool Fader::process(ProcessBuffer* buffer, int64_t steadyTime) {
    for (auto [in, out] : std::views::zip(buffer->_in.buffer32(), buffer->_out.buffer32())) {
        for (auto [a, b] : std::views::zip(in, out)) {
            b = a * _level;
        }
    }
    return true;
}

void Fader::renderContent() {
    ImGui::PushItemWidth(-FLT_MIN);
    ImGui::SliderFloat("##Level", &_level, 0.0f, 2.0f, "Level %.3f");
}
