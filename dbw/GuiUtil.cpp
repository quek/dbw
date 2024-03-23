#include "GuiUtil.h"
#include "Config.h"
#include <algorithm>
#include <imgui_internal.h>

float TEXT_BASE_WIDTH;
float TEXT_BASE_HEIGHT;

ImVec4 COLOR_PALY_LINE = ImVec4(0.3f, 0.7f, 0.7f, 0.65f);
ImVec4 COLOR_BUTTON_ON = ImVec4(0.3f, 0.7f, 0.7f, 0.65f);
ImVec4 COLOR_BUTTON_ON_HOVERED = ImVec4(0.5f, 0.9f, 0.9f, 0.65f);
ImVec4 COLOR_BUTTON_ON_ACTIVE = ImVec4(0.4f, 0.8f, 0.8f, 0.65f);

ImU32 BAR_LINE_COLOR = IM_COL32(0x88, 0x88, 0x88, 0x88);
ImU32 BEAT_LINE_COLOR = IM_COL32(0x55, 0x55, 0x55, 0x88);
ImU32 BEAT16TH_LINE_COLOR = IM_COL32(0x33, 0x33, 0x33, 0x88);
ImU32 PLAY_CURSOR_COLOR = IM_COL32(0x33, 0x33, 0xff, 0xcc);
ImU32 RANGE_SELECTING_COLOR = IM_COL32(0x66, 0x22, 0x22, 0x88);

ImU32 BACKGROUD_WHITE_KEY_COLOR = IM_COL32(0x22, 0x22, 0x22, 0x88);
ImU32 BACKGROUD_BLACK_KEY_COLOR = IM_COL32(0x00, 0x00, 0x00, 0x88);

float widthWithPadding(int nchars) {
    ImGuiStyle& style = ImGui::GetStyle();
    return TEXT_BASE_WIDTH * nchars + style.FramePadding.x * 2;
}

ImU32 selectedColor(const ImU32 color) {
    float weight = 1.3f;
    ImColor x(color);
    return ImColor(
        std::min(x.Value.x * weight, 1.0f),
        std::min(x.Value.y * weight, 1.0f),
        std::min(x.Value.z * weight, 1.0f),
        std::min(x.Value.w * weight, 1.0f)
    );
}

bool operator<(const ImVec2& lhs, const ImVec2& rhs) {
    return lhs.x < rhs.x && lhs.y < rhs.y;
}

bool operator<=(const ImVec2& lhs, const ImVec2& rhs) {
    return lhs.x <= rhs.x && lhs.y <= rhs.y;
}

bool operator>(const ImVec2& lhs, const ImVec2& rhs) {
    return lhs.x > rhs.x && lhs.y > rhs.y;
}

bool operator>=(const ImVec2& lhs, const ImVec2& rhs) {
    return lhs.x >= rhs.x && lhs.y >= rhs.y;
}

Bounds::Bounds() {
}

Bounds::Bounds(const ImVec2& a, const ImVec2& b) {
    p.x = std::min(a.x, b.x);
    p.y = std::min(a.y, b.y);
    q.x = std::max(a.x, b.x);
    q.y = std::max(a.y, b.y);
}

bool Bounds::contains(const ImVec2& pos) const {
    return pos.x >= p.x && pos.x <= q.x
        && pos.y >= p.y && pos.y <= q.y;
}

bool Bounds::overlaped(const Bounds& other) const {
    return p.x <= other.q.x && q.x >= other.p.x
        && p.y <= other.q.y && q.y >= other.p.y;
}

bool defineShortcut(ImGuiKeyChord keyChord, const char* label, const ImVec2& size) {
    ImGui::SetNextItemShortcut(keyChord);
    ImGui::PushID(keyChord);
    bool result = ImGui::Button(label ? label : std::format("##{}", keyChord).c_str(), size);
    ImGui::PopID();
    return result;
}

bool ToggleButton(const char* label, bool* isOn, const ImVec2& size) {
    if (*isOn) {
        ImGui::PushStyleColor(ImGuiCol_Button, gTheme.buttonOn);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, gTheme.buttonOnHovered);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, gTheme.buttonOnActive);
    }
    bool result = ImGui::Button(label, size);
    if (*isOn) {
        ImGui::PopStyleColor(3);
    }
    if (result) {
        *isOn = !*isOn;
    }
    return result;
}
