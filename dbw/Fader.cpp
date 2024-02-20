#include "Fader.h"
#include <ranges>
#include <imgui.h>

Fader::Fader(std::string name, Track* track) : BuiltinModule(name, track) {
}

tinyxml2::XMLElement* Fader::toXml(tinyxml2::XMLDocument* doc) {
    auto* element = doc->NewElement("BuiltinDevice");
    element->SetAttribute("id", xmlId());
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
    if (_mute) {
        std::ranges::fill(buffer->_out[0]._constantp, true);
        for (auto& out : buffer->_out[0].buffer32()) {
            out[0] = 0.0f;
        }
        return true;
    }
    // pan の処理ってどうやるのが正しい？
    float pan = (1.0f - _pan) * 2.0f;
    for (auto [in, out, acp, bcp] : std::views::zip(buffer->_in[0].buffer32(), buffer->_out[0].buffer32(), buffer->_in[0]._constantp, buffer->_out[0]._constantp)) {
        auto in0 = in[0];
        for (auto [a, b] : std::views::zip(in, out)) {
            if (acp && bcp) {
                b = a * _level * pan;
                break;
            } else if (acp) {
                b = in0 * _level * pan;
            } else {
                bcp = false;
                b = a * _level * pan;
            }
        }
        pan = _pan * 2.0f;
    }

    return Module::process(buffer, steadyTime);
}

void Fader::renderContent() {
    ImGui::PushItemWidth(-FLT_MIN);
    ImGui::SliderFloat("##Level", &_level, 0.0f, 2.0f, "Level %.3f");
    ImGui::SliderFloat("##Pan", &_pan, 0.0f, 1.0f, "Pan %.3f");
    ImGui::Checkbox("Mute", &_mute);
    ImGui::SameLine();
    ImGui::Checkbox("Solo", &_solo);
    ImGui::PopItemWidth();
}
