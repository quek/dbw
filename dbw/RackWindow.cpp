#include "RackWindow.h"
#include <ranges>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include "Composer.h"
#include "Config.h"
#include "Error.h"
#include "Fader.h"
#include "SerializeContext.h"
#include "Track.h"
#include "command/GroupTracks.h"
#include "command/AddTrack.h"
#include "command/CopyDragTracks.h"
#include "command/CutTracks.h"
#include "command/DeleteModule.h"
#include "command/DeleteTracks.h"
#include "command/DuplicateTracks.h"
#include "command/MoveTracks.h"
#include "command/PasteTracks.h"

constexpr const float BASE_HEADER_HEIGHT = 18.0f;
constexpr const float GROUP_OFFSET_Y = 5.0f;

RackWindow::RackWindow(Composer* composer) : _composer(composer), _selectedTracks(composer->selectedTracksGet())
{
}

void RackWindow::render()
{
    _allTracks = _composer->allTracks();
    if (ImGui::Begin("Rack", 0, ImGuiWindowFlags_AlwaysHorizontalScrollbar))
    {
        //if (ImGui::BeginChild("Rack Canvas",
        //                      ImVec2(0.0f, 0.0f),
        //                      ImGuiChildFlags_None,
        //                      ImGuiWindowFlags_HorizontalScrollbar))
        //{
        renderHeader();

        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 clipRectMin = windowPos;
        ImVec2 clipRectMax = clipRectMin + ImGui::GetWindowSize() - ImVec2(0.0f, _faderHeight);
        ImGui::PushClipRect(clipRectMin + ImVec2(0.0f, _headerHeight), clipRectMax, true);
        renderModules();
        ImGui::PopClipRect();
        renderFaders();

        //}
        //ImGui::EndChild();
        handleShortcut();
    }
    ImGui::End();
}

void RackWindow::renderHeader()
{
    computeHeaderHeight();

    bool isMaster = true;
    renderHeader(_composer->_masterTrack.get(), 0, isMaster, false);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    auto& style = ImGui::GetStyle();
    ImVec2 pos1 = windowToScreen(ImGui::GetCursorPos()) + ImVec2(-style.ItemSpacing.x / 2.0f - ImGui::GetScrollX(), -style.ItemSpacing.y / 2.0f - ImGui::GetScrollY());
    ImVec2 pos2 = pos1 + ImVec2(0.0f, ImGui::GetWindowHeight());
    drawList->AddLine(pos1, pos2, gTheme.rackBorder);
    if (defineShortcut(ImGuiMod_Ctrl | ImGuiKey_T, "+", ImVec2(0.0f, -FLT_MIN)))
    {
        _composer->commandExecute(new command::AddTrack());
    }
}

void RackWindow::renderHeader(Track* track, int groupLevel, bool isMaster, bool adjustY)
{
    ImGui::PushID(track);
    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();
    float scrollY = ImGui::GetScrollY();
    ImGui::SetCursorPosY(scrollY + GROUP_OFFSET_Y * groupLevel + style.ItemSpacing.y / 2.0f);
    if (isMaster)
    {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + style.ItemSpacing.x / 2.0f);
    }
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 pos1 = windowToScreen(ImGui::GetCursorPos()) - ImVec2(style.ItemSpacing.x / 2.0f + ImGui::GetScrollX(), ImGui::GetScrollY() + style.ItemSpacing.y / 2.0f);
        ImVec2 pos2 = pos1 + ImVec2(0.0f, ImGui::GetWindowHeight());
        drawList->AddLine(pos1, pos2, gTheme.rackBorder);
        pos2 = pos1 + ImVec2(track->_width + style.ItemSpacing.x, 0.0f);
        drawList->AddLine(pos1, pos2, gTheme.rackBorder);
        for (int i = 1; i <= groupLevel; ++i)
        {
            pos1 -= ImVec2(0.0f, GROUP_OFFSET_Y);
            pos2 -= ImVec2(0.0f, GROUP_OFFSET_Y);
            drawList->AddLine(pos1, pos2, gTheme.rackBorder);
        }
    }

    auto it = std::ranges::find(_selectedTracks, track);
    bool selected = it != _selectedTracks.end();
    if (groupLevel > 0 || adjustY)
    {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - style.ItemSpacing.y / 2 - 1.0f);
    }
    ImVec2 size = ImVec2(track->_width, _headerHeight - GROUP_OFFSET_Y * groupLevel - style.ItemSpacing.y);
    if (!isMaster && !track->getTracks().empty())
    {
        size.x -= _groupToggleButtonSize.x;
    }

    if (_renamingTrack == track)
    {
        ImGui::SetKeyboardFocusHere();
        ImGui::SetNextItemWidth(size.x);
        if (ImGui::InputText("##name", &track->_name, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
        {
            _renamingTrack = nullptr;
        }
        if (!ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            _renamingTrack = nullptr;
        }
    }
    else
    {
        // Selectable だとマウスダウンで選択状態にならないのでドラッグしたとき _selectedTracks に入っていないので
        // IsItemClicked を使う
        ImGui::Selectable(track->_name.c_str(), selected, ImGuiSelectableFlags_None, size);
        if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            if (!isMaster)
            {
                if (!io.KeyCtrl && !io.KeyShift)
                {
                    _selectedTracks.clear();
                    _selectedTracks.push_back(track);
                }
                else if (io.KeyCtrl)
                {
                    if (selected)
                    {
                        _selectedTracks.erase(it);
                    }
                    else
                    {
                        _selectedTracks.push_back(track);
                    }
                }
                else if (io.KeyShift)
                {
                    if (_selectedTracks.empty())
                    {
                        for (auto& x : _allTracks)
                        {
                            _selectedTracks.push_back(x);
                            if (x == track) break;
                        }
                    }
                    else
                    {
                        auto from = std::ranges::find(_allTracks, _selectedTracks.back());
                        auto to = std::ranges::find(_allTracks, track);
                        if (from > to)
                        {
                            std::swap(from, to);
                        }
                        while (true)
                        {
                            if (std::ranges::find(_selectedTracks, *from) == _selectedTracks.end())
                            {
                                _selectedTracks.push_back(*from);
                            }
                            if (from++ == to)
                            {
                                break;
                            }
                        }
                    }
                }
            }
        }
        if (!isMaster)
        {
            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Rename", "Ctrl+R"))
                {
                    // TODO
                    _renamingTrack = track;
                }
                if (ImGui::MenuItem("Group", "Ctrl+G"))
                {
                    _composer->commandExecute(new command::GroupTracks(_selectedTracks, true));
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Ungroup", "Ctrl+Shift+G"))
                {
                    // TODO
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Delete", "Delete"))
                {
                    if (!_selectedTracks.empty())
                    {
                        _composer->commandExecute(new command::DeleteTracks(_selectedTracks));
                        _selectedTracks.clear();
                    }
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                _renamingTrack = track;
            }
            if (ImGui::BeginDragDropSource())
            {
                if (!track->included(_selectedTracks))
                {
                    if (!io.KeyCtrl)
                    {
                        _selectedTracks.clear();
                    }
                    _selectedTracks.push_back(track);
                }
                ImGui::SetDragDropPayload("tracks", nullptr, 0);
                std::string text;
                for (auto& x : _selectedTracks)
                {
                    text += x->_name + ", ";
                }
                ImGui::Text(text.c_str());
                ImGui::EndDragDropSource();
            }
            if (!_selectedTracks.empty() && !track->included(_selectedTracks))
            {
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("tracks"))
                    {
                        if (io.KeyCtrl)
                        {
                            _composer->commandExecute(new command::CopyDragTracks(_selectedTracks, track));
                        }
                        else
                        {
                            _composer->commandExecute(new command::MoveTracks(_selectedTracks, track));
                        }
                    }
                }
            }
            if (!track->getTracks().empty())
            {
                ImGui::SameLine();
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() - style.ItemSpacing.x / 2.0f);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + GROUP_OFFSET_Y * groupLevel - 1.0f);
                if (track->_showTracks)
                {
                    if (ImGui::Button("≪", ImVec2(_groupToggleButtonSize.x, size.y + style.ItemSpacing.y / 2.0f + 1.0f)))
                    {
                        track->_showTracks = false;
                    }
                }
                else
                {
                    if (ImGui::Button("≫", ImVec2(_groupToggleButtonSize.x, size.y + style.ItemSpacing.y / 2.0f + 1.0f)))
                    {
                        track->_showTracks = true;
                    }
                }
            }
        }
    }

    // SameLine と SetCurosrPosX の呼び出し順を変えると挙動が変わる
    ImGui::SameLine();
    if (!isMaster && !track->getTracks().empty())
    {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() - style.ItemSpacing.x / 2.0f);
    }

    if (track->_showTracks)
    {
        for (auto& x : track->getTracks())
        {
            renderHeader(x.get(), groupLevel + (isMaster ? 0 : 1), false, adjustY);
            // ImGui::Button("≪", "≫" のあとずれるので、よくわからないけど対処療法
            if (!x->getTracks().empty())
            {
                adjustY = true;
            }
        }
    }
    ImGui::PopID();
}

void RackWindow::renderModules()
{
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetStyle().ItemSpacing.x / 2.0f);
    ImGui::SetCursorPosY(_headerHeight);
    renderModules(_composer->_masterTrack.get());
}

void RackWindow::renderModules(Track* track)
{
    if (track != _composer->_masterTrack.get())
    {
        ImGui::SameLine();
    }

    ImGui::PushID(track);
    ImGui::BeginGroup();
    track->_gain->render(_selectedModules, track->_width);

    auto& style = ImGui::GetStyle();
    float modulesHeightStart = ImGui::GetCursorPosY();
    float modulesHeight = ImGui::GetWindowHeight() - modulesHeightStart - _faderHeight - style.WindowPadding.y * 2 - style.ScrollbarSize;

    ImGuiChildFlags childFlags = ImGuiChildFlags_None;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    if (ImGui::BeginChild("##modules", ImVec2(track->_width, modulesHeight), childFlags, ImGuiWindowFlags_AlwaysVerticalScrollbar))
    {
        float width = track->_width - style.ScrollbarSize;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));
        for (auto& module : track->_modules)
        {
            module->render(_selectedModules, width);
        }
        if (ImGui::Button("Add Module", ImVec2(width, 0.0f)))
        {
            track->_openModuleSelector = true;
        }
        if (track->_openModuleSelector)
        {
            gPluginManager.openModuleSelector(track);
        }
        ImGui::PopStyleVar();
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();



    // TODO とりあえずの余白確保
    //ImGui::InvisibleButton("vscpae", ImVec2(track->_width, 300.0f));

    ImGui::EndGroup();
    ImGui::PopID();
    if (track->_showTracks)
    {
        for (auto& x : track->getTracks())
        {
            ImGui::SameLine();
            renderModules(x.get());
        }
    }
}

void RackWindow::renderFaders()
{
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetStyle().ItemSpacing.x / 2.0f);
    float windowHeight = ImGui::GetWindowHeight();
    auto& style = ImGui::GetStyle();
    float cursorPosY = windowHeight - _faderHeight - style.WindowPadding.y - style.ScrollbarSize;
    if (cursorPosY < _headerHeight)
    {
        cursorPosY = _headerHeight;
    }
    ImGui::SetCursorPosY(cursorPosY);
    // 微妙に縦スクロールするので
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, 0));
    renderFaders(_composer->_masterTrack.get());
    ImGui::PopStyleVar();
}

void RackWindow::renderFaders(Track* track)
{
    if (track != _composer->_masterTrack.get())
    {
        ImGui::SameLine();
    }
    ImGui::BeginGroup();
    track->_fader->render(_selectedModules, track->_width, _faderHeight);
    ImGui::EndGroup();
    if (track->_showTracks)
    {
        for (auto& x : track->getTracks())
        {
            ImGui::SameLine();
            renderFaders(x.get());
        }
    }
}

void RackWindow::handleShortcut()
{
    if (!canHandleInput())
    {
        return;
    }

    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_G))
    {
        if (!_selectedTracks.empty())
        {
            _composer->commandExecute(new command::GroupTracks(_selectedTracks, true));
        }
    }
    else if (ImGui::IsKeyChordPressed(ImGuiKey_Delete))
    {
        if (!_selectedTracks.empty())
        {
            _composer->commandExecute(new command::DeleteTracks(_selectedTracks));
        }
        _selectedTracks.clear();
    }
    else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_C))
    {
        // COPY
        if (!_selectedTracks.empty())
        {
            SerializeContext context;
            nlohmann::json json;
            for (const auto& track : _selectedTracks)
            {
                json["tracks"].push_back(track->toJson(context));
            }
            ImGui::SetClipboardText(json.dump(2).c_str());
            auto renewJson = renewNekoId(json);
            ImGui::SetClipboardText(renewJson.dump(2).c_str());
        }
    }
    else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_X))
    {
        // CUT
        if (!_selectedTracks.empty())
        {
            _composer->commandExecute(new command::CutTracks(_selectedTracks));
            _selectedTracks.clear();
        }
    }
    else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_V))
    {
        // PASTE
        try
        {
            nlohmann::json json = nlohmann::json::parse(ImGui::GetClipboardText());
            if (json.contains("tracks") && json["tracks"].is_array())
            {
                if (!_selectedTracks.empty())
                {
                    _composer->commandExecute(new command::PasteTracks(json["tracks"], _selectedTracks.back()));
                }
            }
        }
        catch (nlohmann::json::exception& e)
        {
            Error("Paste error {}", e.what());
            // ignore
        }
    }

    if (defineShortcut(ImGuiKey_D))
    {
        _composer->commandExecute(new command::DuplicateTracks(_selectedTracks));
    }
    else        if (defineShortcut(ImGuiKey_Delete))
    {
        if (_selectedModules.size() == 1)
        {
            _composer->commandExecute(new command::DeleteModule(_selectedModules[0]));
            _selectedModules.clear();
        }
    }



}

void RackWindow::computeHeaderHeight()
{
    _headerHeight = BASE_HEADER_HEIGHT;
    bool isMaster = true;
    for (auto& track : _allTracks)
    {
        if (isMaster)
        {
            isMaster = false;
        }
        else
        {
            computeHeaderHeight(track, 0);
        }
    }
}

void RackWindow::computeHeaderHeight(Track* track, int groupLevel)
{
    _headerHeight = std::max(_headerHeight, BASE_HEADER_HEIGHT + GROUP_OFFSET_Y * groupLevel);
    if (track->_showTracks)
    {
        for (const auto& x : track->getTracks())
        {
            computeHeaderHeight(x.get(), groupLevel + 1);
        }
    }
}

