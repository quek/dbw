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
    double peakHoldTime = gPreference.sampleRate * 3.0;
    _peakValueLeft = 0.0f;
    _peakValueRight = 0.0f;
    _peakSampleElapsedLeft += buffer->_framesPerBuffer;
    if (_peakSampleElapsedLeft > peakHoldTime)
    {
        _peakValueHoldLeft = std::max(0.0f, _peakValueHoldLeft - 0.02f);
        _peakSampleElapsedLeft -= gPreference.sampleRate * 0.1;
    }
    _peakSampleElapsedRight += buffer->_framesPerBuffer;
    if (_peakSampleElapsedRight > peakHoldTime)
    {
        _peakValueHoldRight = std::max(0.0f, _peakValueHoldRight - 0.02f);
        _peakSampleElapsedRight -= gPreference.sampleRate * 0.1;
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
    float* peakValue = &_peakValueLeft;
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

            if ((*peakValue) < b)
            {
                (*peakValue) = b;
            }
        }
        pan = _pan * 2.0f;
        peakValue = &_peakValueRight;
    }

    if (_peakValueHoldLeft < _peakValueLeft)
    {
        _peakValueHoldLeft = _peakValueLeft;
        _peakSampleElapsedLeft = 0;
    }
    if (_peakValueHoldRight < _peakValueRight)
    {
        _peakValueHoldRight = _peakValueRight;
        _peakSampleElapsedRight = 0;
    }

    return Module::process(buffer, steadyTime);
}

void Fader::render(float width, float height)
{
    ImGui::PushID(this);
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImGuiChildFlags childFlags = ImGuiChildFlags_Border;
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None;
    if (ImGui::BeginChild("##fader", ImVec2(width, height), childFlags, windowFlags))
    {
        ImGui::BeginGroup();

        float windowHeight = ImGui::GetWindowHeight();
        auto& style = ImGui::GetStyle();
        float meterHeight = windowHeight - style.WindowPadding.y * 2;
        float meterWidth = 24.0f;

        ImVec2 meterPos1 = ImGui::GetCursorPos() + ImGui::GetWindowPos();
        ImVec2 meterPos2 = meterPos1 + ImVec2(meterWidth, meterHeight);
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddRect(meterPos1, meterPos2, gTheme.rackBorder);


        ImVec2 pos1 = meterPos1 + ImVec2(0.0f, meterHeight * (1.0f - ZERO_DB_NORMALIZED_VALUE));
        ImVec2 pos2 = pos1 + ImVec2(meterWidth, 0.0f);
        drawList->AddLine(pos1, pos2, gTheme.rackBorder);
        drawList->AddText(pos1 + ImVec2(7.0f, 0.0f), gTheme.text, "0");

        pos1 = meterPos1 + ImVec2(0.0f, meterHeight * (1.0f - dbToLinear(-6.0f)));
        pos2 = pos1 + ImVec2(meterWidth, 0.0f);
        drawList->AddLine(pos1, pos2, gTheme.rackBorder);
        drawList->AddText(pos1 + ImVec2(7.0f, 0.0f), gTheme.text, "6");

        pos1 = meterPos1 + ImVec2(0.0f, meterHeight * (1.0f - dbToLinear(-12.0f)));
        pos2 = pos1 + ImVec2(meterWidth, 0.0f);
        drawList->AddLine(pos1, pos2, gTheme.rackBorder);
        drawList->AddText(pos1 + ImVec2(5.0f, 0.0f), gTheme.text, "12");

        pos1 = meterPos1 + ImVec2(0.0f, meterHeight * (1.0f - dbToLinear(-24.0f)));
        pos2 = pos1 + ImVec2(meterWidth, 0.0f);
        drawList->AddLine(pos1, pos2, gTheme.rackBorder);
        drawList->AddText(pos1 + ImVec2(5.0f, 0.0f), gTheme.text, "24");


        pos1 = meterPos1;
        pos2 = meterPos2 - ImVec2(meterWidth / 2.0f, 0);
        pos1.y += (pos2.y - pos1.y) * (1.0f - std::min(1.0f, _peakValueLeft));
        drawList->AddRectFilled(pos1, pos2, IM_COL32(0x00, 0x80, 0xff, 0x80));

        pos1 = meterPos1 + ImVec2(meterWidth / 2.0f, 0);
        pos2 = meterPos2;
        pos1.y += (pos2.y - pos1.y) * (1.0f - std::min(1.0f, _peakValueRight));
        drawList->AddRectFilled(pos1, pos2, IM_COL32(0x00, 0x80, 0xff, 0x80));

        if (_peakValueHoldLeft > 0.001f)
        {
            pos1 = meterPos1;
            pos1.y += meterHeight * (1.0f - std::min(1.0f, _peakValueHoldLeft)) - 1.0f;
            pos2 = pos1 + ImVec2(meterWidth / 2.0f, 1.0f);
            drawList->AddRectFilled(pos1, pos2, IM_COL32(0xff, 0, 0, 0x80));
        }
        if (_peakValueHoldRight > 0.001f)
        {
            pos1 = meterPos1 + ImVec2(meterWidth / 2.0f, meterHeight * (1.0f - std::min(1.0f, _peakValueHoldRight)) - 1.0f);
            pos2 = pos1 + ImVec2(meterWidth / 2.0f, 1.0f);
            drawList->AddRectFilled(pos1, pos2, IM_COL32(0xff, 0, 0, 0x80));
        }

        {
            float gainRatio = linearToGainRatio(_level);
            float db = normalizedValueToDb(gainRatio);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32_BLACK_TRANS);
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32_BLACK_TRANS);
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32_BLACK_TRANS);
            ImGui::VSliderFloat("##level", ImVec2(meterWidth, meterHeight), &_level, 0.0f, 1.0f, std::format("{:.2}", db).c_str());
            ImGui::PopStyleColor(3);
        }

        ImGui::EndGroup();

        ImGui::SameLine();

        ImGui::BeginGroup();
        ImGui::Text(std::format("{:> 4.4}dB {:> 4.4}dB",
                                normalizedValueToDb(_peakValueHoldLeft),
                                normalizedValueToDb(_peakValueHoldRight)).c_str());
        ImGui::PushItemWidth(-FLT_MIN);
        ImGui::SliderFloat("##Pan", &_pan, 0.0f, 1.0f, "Pan %.3f");
        ToggleButton("M", &_mute);
        ImGui::SameLine();
        ToggleButton("S", &_solo);
        ImGui::Text(std::to_string(_computedLatency).c_str());
        ImGui::PopItemWidth();
        ImGui::EndGroup();
    }
    ImGui::EndChild();
    ImGui::PopID();
}

nlohmann::json Fader::toJson(SerializeContext& context)
{
    nlohmann::json json = BuiltinModule::toJson(context);
    json.update(*this);
    return json;
}

float Fader::gainRatioToLinear(float gainRatio)
{
    float a = 1.0f / (ZERO_DB_NORMALIZED_VALUE * ZERO_DB_NORMALIZED_VALUE);
    float linear = sqrt((gainRatio - 0.00000006f) / a);
    return linear;
}

float Fader::dbToNormalizedValue(float db)
{
    if (db <= -180.0f) return 0.0f;
    float gainRatio = pow(10, db / 20.0f);
    return gainRatio;
}

float Fader::dbToLinear(float db)
{
    return gainRatioToLinear(dbToNormalizedValue(db));
}


float Fader::linearToGainRatio(float linear)
{
    float a = 1.0f / (ZERO_DB_NORMALIZED_VALUE * ZERO_DB_NORMALIZED_VALUE);
    float gainRatio = a * linear * linear + 0.00000006f;
    return gainRatio;
}

float Fader::normalizedValueToDb(float value)
{
    if (value == 0.0f) return -180.0f;
    float dB = 20.0f * log10(value);
    return dB;
}
