#include "ComposerWindow.h"
#include <imgui.h>
#include "../ImGuiFileDialog/ImGuiFileDialog.h"
#include "Composer.h"
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
            gErrorWindow->show("y");
        }
        ImGui::PopStyleColor(3);
    } else {
        if (ImGui::Button("Loop")) {
            _composer->_looping = true;
            gErrorWindow->show("xxxxxxxxxxx\nyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy");
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
    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 4);
    if (ImGui::DragInt("Lines", &_composer->_maxLine, 1.0f, 1, 0x40 * 1000)) {
        _composer->changeMaxLine();
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
            _composer->_project->open(std::filesystem::path(filePath));
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
    std::vector<float> columnWidths;
    // 2 is timeline and master track.
    if (ImGui::BeginChild("##tracksWindow",
                          ImVec2(0, 400),
                          ImGuiChildFlags_ResizeY)) {
        // BeginChild でくるむと右と下が 1.0f 削れるみたいなので
        if (ImGui::BeginTable("tracks", 2 + static_cast<int>(_composer->_tracks.size()), flags, ImVec2(-1.0f, -1.0f))) {
            ImGui::TableSetupScrollFreeze(1, 1);
            // Make the first column not hideable to match our use of TableSetupScrollFreeze()
            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoResize);
            for (auto i = 0; i < _composer->_tracks.size(); ++i) {
                ImGui::TableSetupColumn(_composer->_tracks[i]->_name.c_str(), ImGuiTableColumnFlags_NoResize);
            }
            ImGui::TableSetupColumn(_composer->_masterTrack->_name.c_str(), ImGuiTableColumnFlags_NoResize);

            //ImGui::TableHeadersRow();
            ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
            ImGui::TableSetColumnIndex(0);
            ImGui::TableHeader("#");
            for (auto i = 0; i < _composer->_tracks.size(); ++i) {
                ImGui::TableSetColumnIndex(i + 1);
                ImGui::PushID(i);
                auto name = _composer->_tracks[i]->_name.c_str();
                ImGui::TableHeader(name);
                ImGui::SameLine(ImGui::CalcTextSize(name).x + ImGui::GetStyle().ItemSpacing.x);
                if (ImGui::Button("+")) {
                    _composer->_commandManager.executeCommand(new AddColumnCommand(_composer->_tracks[i].get()));
                }
                if (_composer->_tracks[i]->_ncolumns > 1) {
                    ImGui::SameLine();
                    if (ImGui::Button("-")) {
                        _composer->_commandManager.executeCommand(new DeleteColumnCommand(_composer->_tracks[i].get()));
                    }
                }
                ImGui::PopID();
            }
            ImGui::TableSetColumnIndex(static_cast<int>(_composer->_tracks.size() + 1));
            ImGui::PushID(_composer->_masterTrack.get());
            ImGui::TableHeader(_composer->_masterTrack->_name.c_str());
            ImGui::PopID();

            for (int line = 0; line < _composer->_maxLine; line++) {
                ImGui::TableNextRow();
                if (line == _composer->_playPosition._line) {
                    ImU32 cell_bg_color = ImGui::GetColorU32(COLOR_PALY_LINE);
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, cell_bg_color);
                    if (!_composer->_scrollLock && _composer->_playing) {
                        ImGui::SetScrollHereY(0.5f); // 0.0f:top, 0.5f:center, 1.0f:bottom
                    }
                }
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%02X", line);
                if (line == 0) {
                    columnWidths.push_back(ImGui::GetContentRegionAvail().x);
                }
                for (auto i = 0; i < _composer->_tracks.size(); ++i) {
                    auto track = _composer->_tracks[i].get();
                    ImGui::TableSetColumnIndex(i + 1);
                    track->renderLine(line);
                    if (line == 0) {
                        columnWidths.push_back(ImGui::GetContentRegionAvail().x);
                    }
                }

                ImGui::TableSetColumnIndex(static_cast<int>(_composer->_tracks.size() + 1));
                _composer->_masterTrack->renderLine(line);
                if (line == 0) {
                    columnWidths.push_back(ImGui::GetContentRegionAvail().x);
                }
            }
            if (_racksScrolled) {
                ImGui::SetScrollX(_lastRacksScrollX);
                _racksScrolled = false;
                _lastTracksScrollX = _lastRacksScrollX;
            } else {
                float x = ImGui::GetScrollX();
                _tracksScrolled = _lastTracksScrollX != x;
                _lastTracksScrollX = x;
            }
            ImGui::EndTable();
        }

        ImVec2 windowSize = ImGui::GetWindowSize();
        if (mainWindowSize.y - windowSize.y < 100) {
            ImGui::SetWindowSize(ImVec2(windowSize.x, mainWindowSize.y - 100));
        }
    }
    ImGui::EndChild();

    if (ImGui::BeginTable("racks", 2 + static_cast<int>(_composer->_tracks.size()), flags, ImVec2(-1.0f, -20.0f))) {
        ImGui::TableSetupScrollFreeze(1, 1);
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed, columnWidths[0]);
        for (size_t i = 0; i < _composer->_tracks.size(); ++i) {
            ImGui::TableSetupColumn(_composer->_tracks[i]->_name.c_str(), ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed, columnWidths[i + 1]);
        }
        ImGui::TableSetupColumn(_composer->_masterTrack->_name.c_str(), ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed, columnWidths[_composer->_tracks.size() + 1]);
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
