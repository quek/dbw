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
    float gainRatio = levelToGainRatio(_level);
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

void Fader::render(std::vector<Module*>& selectedModules, float width, float height)
{
    // RackWindow で微妙に縦スクロールするので 0 にしたのを、元に戻す
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f));

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

        ImVec2 meterPos1 = ImGui::GetCursorPos() + ImGui::GetWindowPos() - ImVec2(0.0f, -3.0f);
        ImVec2 meterPos2 = meterPos1 + ImVec2(meterWidth, meterHeight);
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddRect(meterPos1, meterPos2, gTheme.rackBorder);


        ImVec2 pos1 = meterPos1 + ImVec2(0.0f, meterHeight * (1.0f - ZERO_DB_METER));
        ImVec2 pos2 = pos1 + ImVec2(meterWidth, 0.0f);
        drawList->AddLine(pos1, pos2, gTheme.rackBorder);
        drawList->AddText(pos1 + ImVec2(7.0f, 0.0f), gTheme.text, "0");

        pos1 = meterPos1 + ImVec2(0.0f, meterHeight * (1.0f - dbToMeter(-6.0f)));
        pos2 = pos1 + ImVec2(meterWidth, 0.0f);
        drawList->AddLine(pos1, pos2, gTheme.rackBorder);
        drawList->AddText(pos1 + ImVec2(7.0f, 0.0f), gTheme.text, "6");

        pos1 = meterPos1 + ImVec2(0.0f, meterHeight * (1.0f - dbToMeter(-12.0f)));
        pos2 = pos1 + ImVec2(meterWidth, 0.0f);
        drawList->AddLine(pos1, pos2, gTheme.rackBorder);
        drawList->AddText(pos1 + ImVec2(5.0f, 0.0f), gTheme.text, "12");

        pos1 = meterPos1 + ImVec2(0.0f, meterHeight * (1.0f - dbToMeter(-18.0f)));
        pos2 = pos1 + ImVec2(meterWidth, 0.0f);
        drawList->AddLine(pos1, pos2, gTheme.rackBorder);
        drawList->AddText(pos1 + ImVec2(5.0f, 0.0f), gTheme.text, "18");

        pos1 = meterPos1 + ImVec2(0.0f, meterHeight * (1.0f - dbToMeter(-30.0f)));
        pos2 = pos1 + ImVec2(meterWidth, 0.0f);
        drawList->AddLine(pos1, pos2, gTheme.rackBorder);
        drawList->AddText(pos1 + ImVec2(5.0f, 0.0f), gTheme.text, "30");


        pos1 = meterPos1;
        pos2 = meterPos2 - ImVec2(meterWidth / 2.0f, 0);
        pos1.y += (pos2.y - pos1.y) * (1.0f - std::min(1.0f, dbToMeter(normalizedValueToDb(_peakValueLeft))));
        drawList->AddRectFilled(pos1, pos2, IM_COL32(0x00, 0x80, 0xff, 0x80));

        pos1 = meterPos1 + ImVec2(meterWidth / 2.0f, 0);
        pos2 = meterPos2;
        pos1.y += (pos2.y - pos1.y) * (1.0f - std::min(1.0f, dbToMeter(normalizedValueToDb(_peakValueRight))));
        drawList->AddRectFilled(pos1, pos2, IM_COL32(0x00, 0x80, 0xff, 0x80));

        if (_peakValueHoldLeft > 0.001f)
        {
            pos1 = meterPos1;
            pos1.y += meterHeight * (1.0f - std::min(1.0f, normalizedValueToMeter(_peakValueHoldLeft)));
            pos2 = pos1 + ImVec2(meterWidth / 2.0f, 1.0f);
            drawList->AddRectFilled(pos1, pos2, IM_COL32(0xff, 0, 0, 0x80));
        }
        if (_peakValueHoldRight > 0.001f)
        {
            pos1 = meterPos1 + ImVec2(meterWidth / 2.0f, meterHeight * (1.0f - std::min(1.0f, normalizedValueToMeter(_peakValueHoldRight))));
            pos2 = pos1 + ImVec2(meterWidth / 2.0f, 1.0f);
            drawList->AddRectFilled(pos1, pos2, IM_COL32(0xff, 0, 0, 0x80));
        }

        {
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32_BLACK_TRANS);
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32_BLACK_TRANS);
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32_BLACK_TRANS);
            ImGui::PushStyleColor(ImGuiCol_SliderGrab, IM_COL32(0xff, 0xff, 0xff, 0x80));
            ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, IM_COL32(0xff, 0xff, 0xff, 0x80));
            ImGui::SetCursorPosY(ImGui::GetCursorPosY());
            ImGui::VSliderFloat("##level", ImVec2(meterWidth, meterHeight), &_level, 0.0f, 1.0f, "");
            if (ImGui::IsItemActive() || ImGui::IsItemHovered())
            {
                float gainRatio = levelToGainRatio(_level);
                float db = normalizedValueToDb(gainRatio);
                std::string value = db <= -140.0f ? "-∞dB" : std::format("{:4.3}dB", db);
                ImGui::SetTooltip(value.c_str());
            }
            ImGui::PopStyleColor(5);
            if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
            {
                _level = ZERO_DB_SLIDER;
            }
        }

        ImGui::EndGroup();

        ImGui::SameLine();

        ImGui::BeginGroup();
        {
            //auto left = _peakValueHoldLeft < 0.0001f ?
            //    "-∞dB" : std::format("{:> 4.4}dB", normalizedValueToDb(_peakValueHoldLeft));
            //auto right = _peakValueHoldRight < 0.0001f ?
            //    "-∞dB" : std::format("{:> 4.4}dB", normalizedValueToDb(_peakValueHoldRight));
            //ImGui::Text(std::format("{} {}", left, right).c_str());
            //ImGui::Text(std::format("{:.4} {:.4}", _peakValueHoldLeft, _peakValueHoldRight).c_str());
        }
        ImGui::PushItemWidth(-FLT_MIN);
        ImGui::SliderFloat("##Pan", &_pan, 0.0f, 1.0f, "Pan %.3f");
        if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
        {
            _pan = 0.5f;
        }
        ImVec2 size(20.0f, 0.0f);
        ToggleButton("M", &_mute, size);
        ImGui::SameLine();
        ToggleButton("S", &_solo, size);
        {
            float gainRatio = levelToGainRatio(_level);
            float db = normalizedValueToDb(gainRatio);
            ImGui::SetNextItemWidth(45.0f);
            if (ImGui::InputFloat("dB", &db, 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue))
            {
                _level = gainRatioToLevel(dbToNormalizedValue(db));
            }
        }
        ImGui::Text(std::to_string(_computedLatency).c_str());
        ImGui::PopItemWidth();
        ImGui::EndGroup();
    }
    ImGui::EndChild();
    ImGui::PopID();

    ImGui::PopStyleVar();
}

nlohmann::json Fader::toJson(SerializeContext& context)
{
    nlohmann::json json = BuiltinModule::toJson(context);
    json.update(*this);
    return json;
}

float Fader::dbToNormalizedValue(float db)
{
    if (db <= -180.0f) return 0.0f; // -180dB以下は0.0fとする
    float value = pow(10.0f, db / 20.0f);
    return value;
}

float Fader::dbToMeter(float db)
{
    const float dBMin = -180.0f;
    const float dBMax = 12.0f;
    const float targetAtZeroDB = ZERO_DB_METER;

    // 正規化されたdB値 (0.0 から 1.0)
    float normalizedDB = (db - dBMin) / (dBMax - dBMin);

    // 0dBの時の正規化された値
    float normalizedZeroDB = (0.0f - dBMin) / (dBMax - dBMin);

    // エクスポネンシャルカーブの調整
    // 注: この部分は特定のカーブを仮定していますが、
    // 実際の要求に応じて調整が必要です。
    float exponent = log(targetAtZeroDB) / log(normalizedZeroDB);
    float mappedValue = pow(normalizedDB, exponent);

    return mappedValue;
}

float Fader::gainRatioToLevel(float gainRatio)
{
    const float inputMin = 0.0f;
    const float inputMax = 4.0f; // 元の出力範囲が入力範囲に
    const float outputMin = 0.0f;
    const float outputMax = 1.0f; // 元の入力範囲が出力範囲に
    const float targetAtZeroDb = 1.0f;

    // 0.7f の時の正規化された値
    float normalizedPointSeven = (0.7f - outputMin) / (outputMax - outputMin);

    // 出力値1.0に対する正規化された出力範囲内での位置
    float normalizedTarget = (targetAtZeroDb - inputMin) / (inputMax - inputMin);

    // エクスポネンシャルカーブの調整
    float exponent = log(normalizedTarget) / log(normalizedPointSeven);

    // 入力値を正規化（今回はゲイン比を正規化）
    float normalizedInput = (gainRatio - inputMin) / (inputMax - inputMin);

    // 逆マッピングの計算（対数関数を使用）
    float mappedValue = pow(normalizedInput, 1.0f / exponent);

    // 出力範囲に適応（元のレベルを求める）
    float output = mappedValue * (outputMax - outputMin) + outputMin;

    return output;
}

float Fader::levelToGainRatio(float level)
{
    const float inputMin = 0.0f;
    const float inputMax = 1.0f;
    const float outputMin = 0.0f;
    const float outputMax = 4.0f;
    const float targetAtZeroDb = 1.0f;

    // 0.7f の時の正規化された値
    float normalizedPointSeven = (0.7f - inputMin) / (inputMax - inputMin);

    // 出力値1.0に対する正規化された出力範囲内での位置
    float normalizedTarget = (targetAtZeroDb - outputMin) / (outputMax - outputMin);

    // エクスポネンシャルカーブの調整
    // 注: 実際の要求に応じて調整が必要です。
    float exponent = log(normalizedTarget) / log(normalizedPointSeven);

    // 入力値を正規化
    float normalizedInput = (level - inputMin) / (inputMax - inputMin);

    // マッピングされた値を計算
    float mappedValue = pow(normalizedInput, exponent);

    // 出力範囲に適応
    float output = mappedValue * (outputMax - outputMin) + outputMin;

    return output;
}

float Fader::normalizedValueToDb(float value)
{
    if (value == 0.0f) return -180.0f;
    float db = 20.0f * log10(value);
    return db;
}

float Fader::normalizedValueToMeter(float value)
{
    return dbToMeter(normalizedValueToDb(value));
}
