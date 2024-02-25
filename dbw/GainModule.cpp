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
    return json;
}

bool GainModule::process(ProcessBuffer* buffer, int64_t steadyTime) {
    for (auto [in, out, constantp] : std::views::zip(buffer->_in[0].buffer32(),
                                                     buffer->_out[0].buffer32(),
                                                     buffer->_in[0]._constantp)) {
        if (constantp) {
            auto value = in[0] * _gain;
            for (auto b : out) {
                b = value;
            }
        } else {
            for (auto [a, b] : std::views::zip(in, out)) {
                b = a * _gain;
            }
        }
    }
    std::ranges::fill(buffer->_out[0]._constantp, false);
    return Module::process(buffer, steadyTime);
}

void GainModule::renderContent() {
    // TODO undo
    ImGui::PushItemWidth(-FLT_MIN);
    ImGui::SliderFloat("##Gain", &_gain, 0.0f, 2.0f, "Gain %.3f");
}
