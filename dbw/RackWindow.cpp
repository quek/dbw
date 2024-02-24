#include "RackWindow.h"
#include <ranges>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "Composer.h"
#include "Fader.h"
#include "Track.h"

RackWindow::RackWindow(Composer* composer) : _composer(composer) {
}

void RackWindow::render() {
    if (ImGui::Begin("Rack")) {

        if (ImGui::BeginChild("Rack Canvas",
                              ImVec2(0.0f, 0.0f),
                              ImGuiChildFlags_None,
                              ImGuiWindowFlags_HorizontalScrollbar)) {
            renderHeader();

            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 clipRectMin = windowPos;
            ImVec2 clipRectMax = clipRectMin + ImGui::GetWindowSize();
            ImGui::PushClipRect(clipRectMin + ImVec2(0.0f, _headerHeight), clipRectMax, true);
            renderModules();
            renderFaders();
            ImGui::PopClipRect();

            ImGui::EndChild();
        }
    }
    ImGui::End();
}

void RackWindow::renderHeader() {
    float scrollY = ImGui::GetScrollY();
    for (auto& track : _composer->allTracks()) {
        ImGui::SetCursorPosY(scrollY);
        ImGui::Selectable(track->_name.c_str(), &track->_selected, ImGuiSelectableFlags_None, ImVec2(track->_width, TEXT_BASE_HEIGHT));
        ImGui::SameLine();
    }
    if (ImGui::Button("Add track")) {
        _composer->addTrack();
    }
    _headerHeight = TEXT_BASE_HEIGHT;
}

void RackWindow::renderModules() {
    float scrollY = ImGui::GetScrollY();
    bool first = true;
    for (auto& track : _composer->allTracks()) {
        ImGui::SetCursorPosY(_headerHeight);
        if (first) {
            first = false;
        } else {
            ImGui::SameLine();
        }
        ImGui::PushID(track);
        ImGui::BeginGroup();
        for (auto& module : track->_modules) {
            module->render(track->_width);
        }
        if (ImGui::Button("Add Module", ImVec2(track->_width, 0.0f))) {
            track->_openModuleSelector = true;
        }
        if (track->_openModuleSelector) {
            gPluginManager.openModuleSelector(track);
        }
        ImGui::EndGroup();
        ImGui::PopID();
    }
}

void RackWindow::renderFaders() {
    float windowHeight = ImGui::GetWindowHeight();
    float cursorPosY = windowHeight - _faderHeight;
    if (cursorPosY < _headerHeight) {
        cursorPosY = _headerHeight;
    }
    bool first = true;
    for (auto& track : _composer->allTracks()) {
        ImGui::SetCursorPosY(cursorPosY);
        if (first) {
            first = false;
        } else {
            ImGui::SameLine();
        }
        ImGui::BeginGroup();
        track->_fader->render(track->_width, _faderHeight);
        ImGui::EndGroup();
    }
}

