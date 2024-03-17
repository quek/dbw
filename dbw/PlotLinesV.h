#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

namespace ImGui
{
IMGUI_API void PlotLinesV(const char* label, const float* values, int values_count, int values_offset = 0, float scale_min = FLT_MAX, float scale_max = FLT_MAX, ImVec2 graph_size = ImVec2(0, 0), int stride = sizeof(float));
};

