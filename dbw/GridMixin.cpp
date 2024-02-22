#include "GridMixin.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "Grid.h"

GridMixin::GridMixin() {
    _grid = gGrids[1].get();
}

void GridMixin::renderGridSnap() {
    int gridIndex = 0;
    for (; gridIndex < gGrids.size(); ++gridIndex) {
        if (gGrids[gridIndex].get() == _grid) {
            break;
        }
    }
    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 3.5f);
    auto pred = [](void* x, int i) {
        return (*(std::vector<std::unique_ptr<Grid>>*)x)[i]->_name.c_str();
    };
    if (ImGui::Combo("Grid", &gridIndex, pred, &gGrids, static_cast<int>(gGrids.size()))) {
        _grid = gGrids[gridIndex].get();
    }
    ImGui::SameLine();
    ImGui::Checkbox("Snap", &_snap);
}
