#include "GainModule.h"
#include <ranges>
#include "imgui.h"
#include "Composer.h"
#include "Track.h"

GainModule::GainModule(const nlohmann::json& json) : BuiltinModule(json) {
    _gain = json["_gain"];
}

GainModule::GainModule(std::string name, Track* track) :
    BuiltinModule(name, track), _gain(1.0) {
}

nlohmann::json GainModule::toJson() {
    nlohmann::json json = BuiltinModule::toJson();
    json["_id"] = "Gain";
    json["_gain"] = _gain;
    return json;
}

bool GainModule::process(ProcessBuffer* buffer, int64_t steadyTime) {
    for (auto [in, out, inConstantp, outConstantp] : std::views::zip(buffer->_in[0].buffer32(),
                                                     buffer->_out[0].buffer32(),
                                                     buffer->_in[0]._constantp,
                                                     buffer->_out[0]._constantp)) {
        if (inConstantp) {
            out[0] = in[0] * _gain;
            outConstantp = true;
        } else {
            for (auto [a, b] : std::views::zip(in, out)) {
                b = a * _gain;
            }
            outConstantp =false;
        }
    }

    std::swap(buffer->_eventIn, buffer->_eventOut);

    return Module::process(buffer, steadyTime);
}

void GainModule::renderContent() {
    // TODO undo
    ImGui::PushItemWidth(-FLT_MIN);
    ImGui::SliderFloat("##Gain", &_gain, 0.0f, 2.0f, "Gain %.3f");
}
