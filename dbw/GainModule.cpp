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
    for (auto [in, out] : std::views::zip(buffer->_in[0].buffer32(), buffer->_out[0].buffer32())) {
        for (auto [a, b] : std::views::zip(in, out)) {
            b = a * _gain;
        }
    }
    return Module::process(buffer, steadyTime);
}

void GainModule::renderContent() {
    // TODO undo
    ImGui::PushItemWidth(-FLT_MIN);
    ImGui::SliderFloat("##Gain", &_gain, 0.0f, 2.0f, "Gain %.3f");
}
