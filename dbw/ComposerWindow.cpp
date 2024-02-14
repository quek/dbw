#include "ComposerWindow.h"
#include <imgui.h>
#include "../ImGuiFileDialog/ImGuiFileDialog.h"
#include "Composer.h"
#include "Command/OpenProject.h"
#include "ErrorWindow.h"
#include "GuiUtil.h"
#include "util.h"

ComposerWindow::ComposerWindow(Composer* composer) : _composer(composer) {
}

void ComposerWindow::render() {
    ImGui::SetNextWindowSizeConstraints(ImVec2(300, 200), ImVec2(FLT_MAX, FLT_MAX));

    auto composerWindowName = _composer->_project->_name.string() + "##Composer";
    ImGui::Begin(composerWindowName.c_str(), nullptr, ImGuiWindowFlags_NoScrollbar);

    if (_composer->_playing) {
        ImGui::PushStyleColor(ImGuiCol_Button, COLOR_BUTTON_ON);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, COLOR_BUTTON_ON_HOVERED);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, COLOR_BUTTON_ON_ACTIVE);
        if (ImGui::Button("Play")) {
            _composer->stop();
        }
        ImGui::PopStyleColor(3);
    } else {
        if (ImGui::Button("Play")) {
            _composer->play();
        }
    }
    ImGui::SameLine();
    if (_composer->_looping) {
        ImGui::PushStyleColor(ImGuiCol_Button, COLOR_BUTTON_ON);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, COLOR_BUTTON_ON_HOVERED);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, COLOR_BUTTON_ON_ACTIVE);
        if (ImGui::Button("Loop")) {
            _composer->_looping = false;
        }
        ImGui::PopStyleColor(3);
    } else {
        if (ImGui::Button("Loop")) {
            _composer->_looping = true;
        }
    }
    ImGui::SameLine();
    if (_composer->_scrollLock) {
        ImGui::PushStyleColor(ImGuiCol_Button, COLOR_BUTTON_ON);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, COLOR_BUTTON_ON_HOVERED);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, COLOR_BUTTON_ON_ACTIVE);
        if (ImGui::Button("ScLk")) {
            _composer->_scrollLock = false;
        }
        ImGui::PopStyleColor(3);
    } else {
        if (ImGui::Button("ScLk")) {
            _composer->_scrollLock = true;
        }
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 5);
    ImGui::DragFloat("BPM", &_composer->_bpm, 1.0f, 0.0f, 999.0f, "%.02f");
    ImGui::SameLine();
    if (ImGui::Button("Undo")) {
        _composer->_commandManager.undo();
    }
    ImGui::SameLine();
    if (ImGui::Button("Redo")) {
        _composer->_commandManager.redo();
    }
    ImGui::SameLine();
    if (ImGui::Button("Scan Plugin")) {
        _composer->scanPlugin();
    }

    ImGui::SameLine();
    if (ImGui::Button("Open")) {
        IGFD::FileDialogConfig config;
        config.path = _composer->_project->_dir.string();
        config.flags = ImGuiFileDialogFlags_Modal;
        ImGuiFileDialog::Instance()->OpenDialog("Open Project", "Choose File", nullptr, config);
    }
    if (ImGuiFileDialog::Instance()->Display("Open Project")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
            _composer->_commandManager.executeCommand(new command::OpenProject(std::filesystem::path(filePath)));
        }
        ImGuiFileDialog::Instance()->Close();
    }

    ImGui::SameLine();
    if (ImGui::Button("Save")) {
        if (_composer->_project->_isNew) {
            if (_saveWindow == nullptr) {
                _saveWindow = std::make_unique<SaveWindow>(_composer);
            }
            _showSaveWindow = true;
        } else {
            _composer->_project->save();
        }
    }

    ImVec2 mainWindowSize = ImGui::GetWindowSize();

    ImGuiTableFlags flags = ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;

    if (ImGui::BeginTable("racks", 2 + static_cast<int>(_composer->_tracks.size()), flags, ImVec2(-1.0f, -20.0f))) {
        ImGui::TableSetupScrollFreeze(1, 1);
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed);
        for (size_t i = 0; i < _composer->_tracks.size(); ++i) {
            ImGui::TableSetupColumn(_composer->_tracks[i]->_name.c_str(), ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed);
        }
        ImGui::TableSetupColumn(_composer->_masterTrack->_name.c_str(), ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TableHeader("#");
        for (auto i = 0; i < _composer->_tracks.size(); ++i) {
            ImGui::TableSetColumnIndex(i + 1);
            auto name = _composer->_tracks[i]->_name.c_str();
            ImGui::TableHeader(name);
        }

        ImGui::TableSetColumnIndex(0);
        ImGui::TableNextRow();
        for (auto i = 0; i < _composer->_tracks.size(); ++i) {
            ImGui::TableSetColumnIndex(i + 1);
            ImGui::PushID(i);
            _composer->_tracks[i]->render();
            ImGui::PopID();
        }

        ImGui::TableSetColumnIndex(static_cast<int>(_composer->_tracks.size() + 1));
        ImGui::PushID("MASTER RACK");
        _composer->_masterTrack->render();
        ImGui::PopID();

        if (_tracksScrolled) {
            ImGui::SetScrollX(_lastTracksScrollX);
            _tracksScrolled = false;
            _lastRacksScrollX = _lastTracksScrollX;
        } else {
            float x = ImGui::GetScrollX();
            _racksScrolled = _lastRacksScrollX != x;
            _lastRacksScrollX = x;
        }

        ImGui::EndTable();
    }

    if (ImGui::Button("Add track")) {
        _composer->addTrack();
    }

    ImGui::SameLine();
    ImGui::Text(_statusMessage.c_str());

    {
        if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
            if (_composer->_playing) {
                _composer->stop();
            } else {
                _composer->play();
            }
        }
    }

    ImGui::End();

    if (_saveWindow != nullptr) {
        _saveWindow->render();
    }
}

void ComposerWindow::setStatusMessage(std::string message) {
    _statusMessage = message;
}
