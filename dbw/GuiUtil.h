#pragma once
#include <algorithm>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <nlohmann/json.hpp>

constexpr const char* DDP_AUTOMATION_TARGET = "DDP_AMT{}";

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

extern ImU32 BACKGROUD_WHITE_KEY_COLOR;
extern ImU32 BACKGROUD_BLACK_KEY_COLOR;
extern ImU32 NOTE_COLOR;
extern ImU32 SELECTED_NOTE_COLOR;

extern float widthWithPadding(int nchars);

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

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ImVec4, x, y, z, w);

ImU32 selectedColor(const ImU32 color);

bool defineShortcut(ImGuiKeyChord keyChord, const char* label=nullptr, const ImVec2& size =  ImVec2(FLT_MIN, FLT_MIN));
