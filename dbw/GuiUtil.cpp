#include "imgui.h"
#include "GuiUtil.h"

float TEXT_BASE_WIDTH;
float TEXT_BASE_HEIGHT;

ImVec4 COLOR_PALY_LINE = ImVec4(0.3f, 0.7f, 0.7f, 0.65f);
ImVec4 COLOR_BUTTON_ON = ImVec4(0.3f, 0.7f, 0.7f, 0.65f);
ImVec4 COLOR_BUTTON_ON_HOVERED = ImVec4(0.5f, 0.9f, 0.9f, 0.65f);
ImVec4 COLOR_BUTTON_ON_ACTIVE = ImVec4(0.4f, 0.8f, 0.8f, 0.65f);

float widthWithPadding(int nchars) {
    ImGuiStyle& style = ImGui::GetStyle();
    return TEXT_BASE_WIDTH * nchars + style.FramePadding.x * 2;
}

ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) {
    return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y);
}

ImVec2& operator+=(ImVec2& lhs, const ImVec2& rhs) {
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    return lhs;
}

ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) {
    return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y);
}

ImVec2 operator*(const ImVec2& lhs, const ImVec2& rhs) {
    return ImVec2(lhs.x * rhs.x, lhs.y * rhs.y);
}

bool operator==(const ImVec2& lhs, const ImVec2& rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

bool operator!=(const ImVec2& lhs, const ImVec2& rhs) {
    return !(lhs == rhs);
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
