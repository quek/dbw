#include "PianoRoll.h"
#include <memory>
#include <imgui.h>

std::unique_ptr<PianoRoll> gPianoRoll = nullptr;

ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) {
    return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y);
}

ImVec2& operator+=(ImVec2& lhs, const ImVec2& rhs) {
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    return lhs;
}

void PianoRoll::render() {
    if (!_show) return;

    if (ImGui::Begin("Piano Roll", &_show)) {
        ImGui::Button("Snap");
        ImGui::SetNextItemWidth(1024.0f);
        if (ImGui::BeginChild("##Piano Roll Inner",
                              ImVec2(0, 0),
                              ImGuiChildFlags_None,
                              ImGuiWindowFlags_HorizontalScrollbar)) {
            float windowWidth = ImGui::GetWindowWidth();
            ImVec2 cra = ImGui::GetContentRegionAvail();
            float windowHeight = ImGui::GetWindowHeight();
            ImVec2 windowPos = ImGui::GetWindowPos();
            ImGuiStyle style = ImGui::GetStyle();
            // トップウインドウのとき用
            //ImVec2 offset = ImVec2(windowPos.x + style.FramePadding.x, windowPos.y + windowHeight - cra.y - style.WindowPadding.y * 2);
            ImVec2 offset = ImVec2(windowPos.x - ImGui::GetScrollX(), windowPos.y - ImGui::GetScrollY());
            ImDrawList* drawList = ImGui::GetWindowDrawList();

            //ImGui::InvisibleButton("##dummy InvisibleButton", ImVec2(2100, 1100));




            drawList->AddRectFilled(ImVec2(0, 0) + offset, ImVec2(2000, 1000) + offset, IM_COL32(0, 0, 0x44, 0xff));
            drawList->AddRectFilled(ImVec2(1, 1) + offset, ImVec2(100, 100) + offset, IM_COL32(0, 0x88, 0, 0xff));
            drawList->AddRectFilled(ImVec2(200, 0) + offset, ImVec2(100, 100) + offset, IM_COL32(0, 0x88, 0, 0xff));
            drawList->AddRectFilled(ImVec2(0, 100) + offset, ImVec2(99, 200) + offset, IM_COL32(0, 0x88, 0, 0xff));
            drawList->AddRectFilled(ImVec2(2, 200) + offset, ImVec2(99, 300) + offset, IM_COL32(0, 0x88, 0, 0xff));
            drawList->AddText(ImVec2(15, 25) + offset, IM_COL32_WHITE, "nya~~");
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void PianoRoll::edit(Clip* clip) {
    _clip = clip;
    _show = true;
}
