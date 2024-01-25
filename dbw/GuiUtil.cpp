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

