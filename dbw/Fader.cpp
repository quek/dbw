#include "Fader.h"
#include <ranges>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "Track.h"

Fader::Fader(const nlohmann::json& json) : BuiltinModule(json) {
    _level = json["_level"];
    _pan = json["_pan"];
    _mute = json["_mute"];
    _solo = json["_solo"];
}

Fader::Fader(std::string name, Track* track) : BuiltinModule(name, track) {
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
    ImGui::Text(std::to_string(_track->_latency).c_str());
    ImGui::PopItemWidth();
}

nlohmann::json Fader::toJson() {
    nlohmann::json json = BuiltinModule::toJson();
    json.update(*this);
    return json;
}
