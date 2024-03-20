#include "Fader.h"
#include <ranges>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "Config.h"
#include "Track.h"

Fader::Fader(const nlohmann::json& json, SerializeContext& context) : BuiltinModule(json, context)
{
    _level = json["_level"];
    _pan = json["_pan"];
    _mute = json["_mute"];
    _solo = json["_solo"];
}

Fader::Fader(std::string name, Track* track) : BuiltinModule(name, track)
{
}

bool Fader::process(ProcessBuffer* buffer, int64_t steadyTime)
{
    double peakHoldTime = gPreference.sampleRate * 3000 / 1000;
    _peakSampleElapsed += buffer->_framesPerBuffer;
    if (_peakSampleElapsed > peakHoldTime)
    {
        _peakValue = std::max(0.0f, _peakValue - 0.1f);
    }

    if (_mute)
    {
        std::ranges::fill(buffer->_out[0]._constantp, true);
        for (auto& out : buffer->_out[0].buffer32())
        {
            out[0] = 0.0f;
        }
        return true;
    }
    // pan の処理ってどうやるのが正しい？
    float pan = (1.0f - _pan) * 2.0f;
    float gainRatio = linearToGainRatio(_level);
    for (auto [in, out, acp, bcp] : std::views::zip(buffer->_in[0].buffer32(), buffer->_out[0].buffer32(), buffer->_in[0]._constantp, buffer->_out[0]._constantp))
    {
        auto in0 = in[0];
        for (auto [a, b] : std::views::zip(in, out))
        {
            if (acp && bcp)
            {
                b = a * gainRatio * pan;
                break;
            }
            else if (acp)
            {
                b = in0 * gainRatio * pan;
            }
            else
            {
                bcp = false;
                b = a * gainRatio * pan;
            }

            if (_peakValue < b)
            {
                _peakValue = b;
                _peakSampleElapsed = 0;
            }
        }
        pan = _pan * 2.0f;
    }

    return Module::process(buffer, steadyTime);
}

void Fader::renderContent()
{

    ImGui::BeginGroup();
    ImVec2 pos1 = ImGui::GetCursorPos() + ImGui::GetWindowPos();
    ImVec2 pos2 = pos1 + ImVec2(10.0f, 100.0f);
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(pos1, pos2, IM_COL32_BLACK);
    pos1.y += (pos2.y - pos1.y) * _peakValue;
    drawList->AddRectFilled(pos1, pos2, IM_COL32_WHITE);
    ImGui::VSliderFloat("VU", ImVec2(5.0f, 100.0f), &_level, 0.0f, 1.0f);
    ImGui::EndGroup();

    ImGui::SameLine();

    ImGui::BeginGroup();
    //ImGui::PushItemWidth(-FLT_MIN);
    ImGui::SliderFloat("##Level", &_level, 0.0f, 1.0f, "Level %.3f");
    ImGui::SliderFloat("##Pan", &_pan, 0.0f, 1.0f, "Pan %.3f");
    ImGui::Checkbox("Mute", &_mute);
    ImGui::SameLine();
    ImGui::Checkbox("Solo", &_solo);
    ImGui::Text(std::to_string(_computedLatency).c_str());
    //ImGui::PopItemWidth();
    ImGui::EndGroup();

    ImGui::Text(std::format("{} {}", linearToGainRatio(_level), gainRatioToDB(linearToGainRatio(_level))).c_str());
}

nlohmann::json Fader::toJson(SerializeContext& context)
{
    nlohmann::json json = BuiltinModule::toJson(context);
    json.update(*this);
    return json;
}

float Fader::gainRatioToDB(float gainRatio)
{
    if (gainRatio == 0.0f) return -100.0f;
    float dB = 20.0f * log10(gainRatio);
    return dB;
}

float Fader::linearToGainRatio(float linearValue)
{
    const float minDb = -100.0f;
    const float maxDb = 12.0f;

    // 0.0を-∞ dBに直接マッピングするための対策
    if (linearValue <= 0.0f) return 0.0f;

    // リニア値をゲイン比に変換
    float gainRatio = linearValue * (pow(10.0f, maxDb / 20.0f) - pow(10.0f, minDb / 20.0f)) + pow(10.0f, minDb / 20.0f);

    return gainRatio;
}

