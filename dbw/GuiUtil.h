#pragma once
#include <algorithm>
#include <imgui.h>

extern float TEXT_BASE_WIDTH;
extern float TEXT_BASE_HEIGHT;

extern ImVec4 COLOR_PALY_LINE;
extern ImVec4 COLOR_BUTTON_ON;
extern ImVec4 COLOR_BUTTON_ON_HOVERED;
extern ImVec4 COLOR_BUTTON_ON_ACTIVE;

extern ImU32 BAR_LINE_COLOR;
extern ImU32 BEAT_LINE_COLOR;
extern ImU32 BEAT16TH_LINE_COLOR;
extern ImU32 PLAY_CURSOR_COLOR;
extern ImU32 RANGE_SELECTING_COLOR;

extern float widthWithPadding(int nchars);

ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs);
ImVec2& operator+=(ImVec2& lhs, const ImVec2& rhs);
ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs);
ImVec2 operator* (const ImVec2& lhs, const ImVec2& rhs);
bool operator==(const ImVec2& lhs, const ImVec2& rhs);
bool operator!=(const ImVec2& lhs, const ImVec2& rhs);
bool operator<(const ImVec2& lhs, const ImVec2& rhs);
bool operator<=(const ImVec2& lhs, const ImVec2& rhs);
bool operator>(const ImVec2& lhs, const ImVec2& rhs);
bool operator>=(const ImVec2& lhs, const ImVec2& rhs);

struct Bounds {
    ImVec2 p;
    ImVec2 q;
    Bounds();
    Bounds(const ImVec2& a, const ImVec2& b);
    bool contains(const ImVec2& pos) const;
    bool overlaped(const Bounds& other) const;
};
