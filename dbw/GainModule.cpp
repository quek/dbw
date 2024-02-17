#include "GainModule.h"
#include <ranges>
#include "imgui.h"
#include "Composer.h"
#include "Track.h"

GainModule::GainModule(std::string name, Track* track) :
    BuiltinModule(name, track), _gain(1.0) {
}

tinyxml2::XMLElement* GainModule::toXml(tinyxml2::XMLDocument* doc) {
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
    for (auto [in, out] : std::views::zip(buffer->_in.buffer32(), buffer->_out.buffer32())) {
        for (auto [a, b] : std::views::zip(in, out)) {
            b = a * _gain;
        }
    }
    return true;
}

void GainModule::renderContent() {
    // TODO undo
    ImGui::PushItemWidth(-FLT_MIN);
    ImGui::SliderFloat("##Gain", &_gain, 0.0f, 2.0f, "Gain %.3f");
}
