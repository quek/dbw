#include "Line.h"
#include "Track.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "imgui.h"
#include "Column.h"
#include "Composer.h"
#include "GuiUtil.h"

Line::Line(Track* track, size_t ncolumns) : _track(track) {
    for (size_t i = 0; i < ncolumns; ++i) {
        _columns.push_back(std::make_unique<Column>(this));
    }
}

Line::Line(const char* note, unsigned char velocity, unsigned char delay, Track* track) :
    _track(track) {
    _columns.push_back(std::make_unique<Column>(note, velocity, delay, this));
}

void Line::render() {
    ImGui::PushID(this);
    bool first = true;
    for (auto column = _columns.begin(); column != _columns.end(); ++column) {
        if (first) {
            first = false;
        } else {
            ImGui::SameLine();
        }
        (*column)->render();
    }
    ImGui::PopID();
}
