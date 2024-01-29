#include "GainModule.h"
#include <ranges>
#include "imgui.h"

GainModule::GainModule(std::string name, Track* track) :
    BuiltinModule(name, track), _gain(1.0) {
}

tinyxml2::XMLElement* GainModule::dawProject(tinyxml2::XMLDocument* doc) {
    auto* element = doc->NewElement("BuiltinDevice");
    element->SetAttribute("deviceRole", "audioFX");
    element->SetAttribute("deviceName", _name.c_str());
    element->SetAttribute("deviceID", "Gain");
    auto* parameters = element->InsertNewChildElement("Parameters");
    auto* realParameter = parameters->InsertNewChildElement("RealParameter");
    realParameter->SetAttribute("name", "Gain");
    realParameter->SetAttribute("unit", "linear");
    realParameter->SetAttribute("value", _gain);

    return element;
}

void GainModule::loadParameters(tinyxml2::XMLElement* element) {
    auto param = element->FirstChildElement("RealParameter");
    param->QueryFloatAttribute("value", &_gain);
}

bool GainModule::process(ProcessBuffer* buffer, int64_t /*steadyTime*/) {
    for (auto [in, out] : std::views::zip(buffer->_in._buffer, buffer->_out._buffer)) {
        for (auto [a, b] : std::views::zip(in, out)) {
            b = a * _gain;
        }
    }
    return true;
}

void GainModule::render() {
    ImGui::Text(_name.c_str());
    // TODO undo
    ImGui::PushItemWidth(-FLT_MIN);
    ImGui::SliderFloat("##Gain", &_gain, 0.0f, 2.0f, "Gain %.3f");
}
