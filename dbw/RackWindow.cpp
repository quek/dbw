#include "RackWindow.h"
#include <ranges>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "Composer.h"
#include "Config.h"
#include "Fader.h"
#include "Track.h"
#include "command/GroupTracks.h"
#include "command/AddTrack.h"

constexpr const float BASE_HEADER_HEIGHT = 18.0f;
constexpr const float GROUP_OFFSET_Y = 5.0f;

RackWindow::RackWindow(Composer* composer) : _composer(composer) {
}

void RackWindow::render() {
    _allTracks = _composer->allTracks();
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
        handleShortcut();
    }
    ImGui::End();
}

void RackWindow::renderHeader() {
    computeHeaderHeight();

    bool isMaster = true;
    for (auto& track : _allTracks) {
        renderHeader(track, 0, isMaster);
        isMaster = false;
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    auto& style = ImGui::GetStyle();
    ImVec2 pos1 = windowToScreen(ImGui::GetCursorPos()) + ImVec2(-style.ItemSpacing.x / 2.0f - ImGui::GetScrollX(), -style.ItemSpacing.y / 2.0f - ImGui::GetScrollY());
    ImVec2 pos2 = pos1 + ImVec2(0.0f, ImGui::GetWindowHeight());
    drawList->AddLine(pos1, pos2, gTheme.rackBorder);
    if (ImGui::Button("+", ImVec2(0.0f, -FLT_MIN))) {
        _composer->_commandManager.executeCommand(new command::AddTrack());
    }
}

void RackWindow::renderHeader(Track* track, int groupLevel, bool isMaster) {
    ImGui::PushID(track);
    ImGuiStyle& style = ImGui::GetStyle();
    float scrollY = ImGui::GetScrollY();
    ImGui::SetCursorPosY(scrollY + 5 * groupLevel + style.ItemSpacing.y / 2.0f);
    if (isMaster) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + style.ItemSpacing.x / 2.0f);
    }
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 pos1 = windowToScreen(ImGui::GetCursorPos()) - ImVec2(style.ItemSpacing.x / 2.0f + ImGui::GetScrollX(), ImGui::GetScrollY() + style.ItemSpacing.y / 2.0f);
        ImVec2 pos2 = pos1 + ImVec2(0.0f, ImGui::GetWindowHeight());
        drawList->AddLine(pos1, pos2, gTheme.rackBorder);
        pos2 = pos1 + ImVec2(track->_width + style.ItemSpacing.x, 0.0f);
        drawList->AddLine(pos1, pos2, gTheme.rackBorder);
        for (int i = 1; i <= groupLevel; ++i) {
            pos1 -= ImVec2(0.0f, GROUP_OFFSET_Y);
            pos2 -= ImVec2(0.0f, GROUP_OFFSET_Y);
            drawList->AddLine(pos1, pos2, gTheme.rackBorder);
        }
    }

    auto it = std::ranges::find(_selectedTracks, track);
    bool selected = it != _selectedTracks.end();
    if (groupLevel > 0) {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - style.ItemSpacing.y / 2 - 1.0f);
    }
    ImVec2 size = ImVec2(track->_width, _headerHeight - GROUP_OFFSET_Y * groupLevel - style.ItemSpacing.y);
    if (!track->getTracks().empty()) {
        //size.x -= _groupToggleButtonSize.x + style.ItemSpacing.x / 2.0f;
        size.x -= _groupToggleButtonSize.x;
    }
    if (ImGui::Selectable(track->_name.c_str(), selected, ImGuiSelectableFlags_None, size)) {
        auto& io = ImGui::GetIO();
        if (!io.KeyCtrl && !io.KeyShift) {
            _selectedTracks = { track };
        } else if (io.KeyCtrl) {
            if (selected) {
                _selectedTracks.erase(it);
            } else {
                _selectedTracks.push_back(track);
            }
        } else if (io.KeyShift) {
            if (_selectedTracks.empty()) {
                for (auto& x : _allTracks) {
                    _selectedTracks.push_back(x);
                    if (x == track) break;
                }
            } else {
                auto from = std::ranges::find(_allTracks, _selectedTracks.back());
                auto to = std::ranges::find(_allTracks, track);
                if (from > to) {
                    std::swap(from, to);
                }
                while (true) {
                    if (std::ranges::find(_selectedTracks, *from) == _selectedTracks.end()) {
                        _selectedTracks.push_back(*from);
                    }
                    if (from++ == to) {
                        break;
                    }
                }
            }
        }
    }
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Group", "Ctrl+G")) {
            _composer->_commandManager.executeCommand(new command::GroupTracks(_selectedTracks, true));
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem("Ungroup", "Ctrl+Shift+G")) {
            // TODO
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem("Delete", "Delete"))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    if (!track->getTracks().empty()) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() - style.ItemSpacing.x / 2.0f);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + GROUP_OFFSET_Y * groupLevel - 1.0f);
        if (track->_showTracks) {
            if (ImGui::Button("≪", ImVec2(_groupToggleButtonSize.x, size.y + style.ItemSpacing.y / 2.0f + 1.0f))) {
                track->_showTracks = false;
            }
        } else {
            if (ImGui::Button("≫", ImVec2(_groupToggleButtonSize.x, size.y + style.ItemSpacing.y / 2.0f + 1.0f))) {
                track->_showTracks = true;
            }
        }
        ImGui::SameLine();
    }
    ImGui::SameLine();
    if (!track->getTracks().empty()) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() - style.ItemSpacing.x / 2.0f);
    }

    if (track->_showTracks) {
        for (auto& x : track->getTracks()) {
            renderHeader(x.get(), groupLevel + 1, false);
        }
    }
    ImGui::PopID();
}

void RackWindow::renderModules() {
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetStyle().ItemSpacing.x / 2.0f);
    ImGui::SetCursorPosY(_headerHeight);
    bool first = true;
    for (auto& track : _allTracks) {
        if (first) {
            first = false;
        } else {
            ImGui::SameLine();
        }
        renderModules(track);
    }
}

void RackWindow::renderModules(Track* track) {
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
    if (track->_showTracks) {
        for (auto& x : track->getTracks()) {
            ImGui::SameLine();
            renderModules(x.get());
        }
    }
}

void RackWindow::renderFaders() {
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetStyle().ItemSpacing.x / 2.0f);
    float windowHeight = ImGui::GetWindowHeight();
    float cursorPosY = windowHeight - _faderHeight;
    if (cursorPosY < _headerHeight) {
        cursorPosY = _headerHeight;
    }
    bool first = true;
    for (auto& track : _allTracks) {
        ImGui::SetCursorPosY(cursorPosY);
        if (first) {
            first = false;
        } else {
            ImGui::SameLine();
        }
        renderFaders(track);
    }
}

void RackWindow::renderFaders(Track* track) {
    ImGui::BeginGroup();
    track->_fader->render(track->_width, _faderHeight);
    ImGui::EndGroup();
    if (track->_showTracks) {
        for (auto& x : track->getTracks()) {
            ImGui::SameLine();
            renderFaders(x.get());
        }
    }
}

void RackWindow::handleShortcut() {
    if (!canHandleInput()) {
        return;
    }

    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_G)) {
        if (!_selectedTracks.empty()) {
            _composer->_commandManager.executeCommand(new command::GroupTracks(_selectedTracks, true));
        }
    }
}

void RackWindow::computeHeaderHeight() {
    _headerHeight = BASE_HEADER_HEIGHT;
    for (auto& track : _allTracks) {
        computeHeaderHeight(track, 0);
    }
}

void RackWindow::computeHeaderHeight(Track* track, int groupLevel) {
    _headerHeight = std::max(_headerHeight, BASE_HEADER_HEIGHT + GROUP_OFFSET_Y * groupLevel);
    if (track->_showTracks) {
        for (const auto& x : track->getTracks()) {
            computeHeaderHeight(x.get(), groupLevel + 1);
        }
    }

}

