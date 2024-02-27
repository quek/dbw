#include "RackWindow.h"
#include <ranges>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include "Composer.h"
#include "Config.h"
#include "Error.h"
#include "Fader.h"
#include "Track.h"
#include "command/GroupTracks.h"
#include "command/AddTrack.h"
#include "command/PasteTracks.h"

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
    renderHeader(_composer->_masterTrack.get(), 0, isMaster, false);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    auto& style = ImGui::GetStyle();
    ImVec2 pos1 = windowToScreen(ImGui::GetCursorPos()) + ImVec2(-style.ItemSpacing.x / 2.0f - ImGui::GetScrollX(), -style.ItemSpacing.y / 2.0f - ImGui::GetScrollY());
    ImVec2 pos2 = pos1 + ImVec2(0.0f, ImGui::GetWindowHeight());
    drawList->AddLine(pos1, pos2, gTheme.rackBorder);
    if (ImGui::Button("+", ImVec2(0.0f, -FLT_MIN))) {
        _composer->_commandManager.executeCommand(new command::AddTrack());
    }
}

void RackWindow::renderHeader(Track* track, int groupLevel, bool isMaster, bool adjustY) {
    ImGui::PushID(track);
    ImGuiStyle& style = ImGui::GetStyle();
    float scrollY = ImGui::GetScrollY();
    ImGui::SetCursorPosY(scrollY + GROUP_OFFSET_Y * groupLevel + style.ItemSpacing.y / 2.0f);
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
    if (groupLevel > 0 || adjustY) {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - style.ItemSpacing.y / 2 - 1.0f);
    }
    ImVec2 size = ImVec2(track->_width, _headerHeight - GROUP_OFFSET_Y * groupLevel - style.ItemSpacing.y);
    if (!isMaster && !track->getTracks().empty()) {
        size.x -= _groupToggleButtonSize.x;
    }

    if (_renamingTrack == track) {
        ImGui::SetKeyboardFocusHere();
        ImGui::SetNextItemWidth(size.x);
        if (ImGui::InputText("##name", &track->_name, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue)) {
            _renamingTrack = nullptr;
        }
        if (!ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            _renamingTrack = nullptr;
        }
    } else {
        if (ImGui::Selectable(track->_name.c_str(), selected, ImGuiSelectableFlags_None, size)) {
            if (!isMaster) {
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
        }
        if (!isMaster) {
            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Rename", "Ctrl+R")) {
                    // TODO
                    _renamingTrack = track;
                }
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
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                _renamingTrack = track;
            }
            if (ImGui::BeginDragDropSource()) {
                ImGui::SetDragDropPayload("tracks", nullptr, 0);
                ImGui::EndDragDropSource();
            }
            if (!track->getTracks().empty()) {
                ImGui::SameLine();
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
            }
        }
    }

    // SameLine と SetCurosrPosX の呼び出し順を変えると挙動が変わる
    ImGui::SameLine();
    if (!isMaster && !track->getTracks().empty()) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() - style.ItemSpacing.x / 2.0f);
    }

    if (track->_showTracks) {
        for (auto& x : track->getTracks()) {
            renderHeader(x.get(), groupLevel + (isMaster ? 0 : 1), false, adjustY);
            // ImGui::Button("≪", "≫" のあとずれるので、よくわからないけど対処療法
            if (!x->getTracks().empty()) {
                adjustY = true;
            }
        }
    }
    ImGui::PopID();
}

void RackWindow::renderModules() {
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetStyle().ItemSpacing.x / 2.0f);
    ImGui::SetCursorPosY(_headerHeight);
    renderModules(_composer->_masterTrack.get());
}

void RackWindow::renderModules(Track* track) {
    if (track != _composer->_masterTrack.get()) {
        ImGui::SameLine();
    }

    ImGui::PushID(track);
    ImGui::BeginGroup();
    track->_gain->render(track->_width);
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
    ImGui::SetCursorPosY(cursorPosY);
    renderFaders(_composer->_masterTrack.get());
}

void RackWindow::renderFaders(Track* track) {
    if (track != _composer->_masterTrack.get()) {
        ImGui::SameLine();
    }
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
    } else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_C)) {
        // COPY
        if (!_selectedTracks.empty()) {
            nlohmann::json json;
            for (const auto& track : _selectedTracks) {
                json["tracks"].push_back(track->toJson());
            }
            ImGui::SetClipboardText(json.dump(2).c_str());
            auto renewJson = renewNekoId(json);
            ImGui::SetClipboardText(renewJson.dump(2).c_str());
        }
    } else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_X)) {
        // CUT
    } else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_V)) {
        // PASTE
        try {
            nlohmann::json json = nlohmann::json::parse(ImGui::GetClipboardText());
            if (json.contains("tracks") && json["tracks"].is_array()) {
                if (!_selectedTracks.empty()) {
                    _composer->_commandManager.executeCommand(new command::PasteTracks(json["tracks"], _selectedTracks.back()));
                }
            }
        } catch (nlohmann::json::exception& e) {
            Error("Paste error {}", e.what());
            // ignore
        }
    }
}

void RackWindow::computeHeaderHeight() {
    _headerHeight = BASE_HEADER_HEIGHT;
    bool isMaster = true;
    for (auto& track : _allTracks) {
        if (isMaster) {
            isMaster = false;
        } else {
            computeHeaderHeight(track, 0);
        }
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

