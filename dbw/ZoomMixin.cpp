#include "ZoomMixin.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

ZoomMixin::ZoomMixin(float zoomX, float zoomY) : _zoomX(zoomX), _zoomY(zoomY) {
}

ZoomMixin::~ZoomMixin() {
}

void ZoomMixin::renderDebugZoomSlider() {
    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 5);
    ImGui::DragFloat("Zoom X", &_zoomX, 0.01f, 0.001f);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 5);
    ImGui::DragFloat("Zoom Y", &_zoomY, 0.01f, 0.001f);
}
