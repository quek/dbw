#include "GuiUtil.h"
#include <algorithm>

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
ImU32 NOTE_COLOR = IM_COL32(0x00, 0xcc, 0xcc, 0x88);
ImU32 SELECTED_NOTE_COLOR = IM_COL32(0x66, 0x66, 0xff, 0x88);

float widthWithPadding(int nchars) {
    ImGuiStyle& style = ImGui::GetStyle();
    return TEXT_BASE_WIDTH * nchars + style.FramePadding.x * 2;
}

ImVec4 selectedColor(const ImVec4& color) {
    float weight = 1.3f;
    return ImVec4(
        std::min(color.x * weight, 1.0f),
        std::min(color.y * weight, 1.0f),
        std::min(color.z * weight, 1.0f),
        std::min(color.w * weight, 1.0f)
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
