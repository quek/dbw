#include "ComposerWindow.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "../ImGuiFileDialog/ImGuiFileDialog.h"
#include "AudioEngine.h"
#include "Composer.h"
#include "Command/OpenProject.h"
#include "Config.h"
#include "ErrorWindow.h"
#include "GuiUtil.h"
#include "util.h"

ComposerWindow::ComposerWindow(Composer* composer) : _composer(composer) {
    if (gPreference.audioDeviceIndex == -1) {
        _audioEngineWindow.reset(new AudioEngineWindow(_composer));
        _showAudioEngineWindow = true;
    }
}

void ComposerWindow::render() {
    ImGui::SetNextWindowSizeConstraints(ImVec2(300, 200), ImVec2(FLT_MAX, FLT_MAX));

    // TODO imgui.ini 問題
    //auto composerWindowName = _composer->_project->_name.string() + "##Composer";
    ImGui::Begin("Composer", nullptr, ImGuiWindowFlags_NoScrollbar);

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
        gPluginManager.scan();
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

    ImGui::SameLine();
    if (ImGui::Button("AE")) {
        _audioEngineWindow.reset(new AudioEngineWindow(_composer));
        _showAudioEngineWindow = true;
    }

    ImGui::SameLine();
    ImGui::Text("cpu %.02f", _composer->_audioEngine->_cpuLoad);

    ImVec2 mainWindowSize = ImGui::GetWindowSize();

    ImGuiTableFlags flags = ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;

    if (ImGui::BeginTable("racks", 1 + static_cast<int>(_composer->_tracks.size()), flags, ImVec2(-1.0f, -20.0f))) {
        Track* masterTrack = _composer->_masterTrack.get();
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn(masterTrack->_name.c_str());
        for (size_t i = 0; i < _composer->_tracks.size(); ++i) {
            ImGui::TableSetupColumn(_composer->_tracks[i]->_name.c_str());
        }
        ImGui::TableHeadersRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TableHeader(masterTrack->_name.c_str());
        if (ImGui::IsItemActivated()) {
            _selectedTracks.clear();
            _selectedTracks.insert(masterTrack);
        }
        ImVec4 colorMasterTrack = _selectedTracks.contains(masterTrack) ? selectedColor(masterTrack->_color) : masterTrack->_color;
        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(colorMasterTrack));

        for (auto i = 0; i < _composer->_tracks.size(); ++i) {
            Track* track = _composer->_tracks[i].get();
            ImGui::TableSetColumnIndex(i + 1);
            ImGui::TableHeader(track->_name.c_str());
            if (ImGui::IsItemActivated()) {
                _selectedTracks.clear();
                _selectedTracks.insert(track);
            }
            ImVec4 color = _selectedTracks.contains(track) ? selectedColor(track->_color) : track->_color;
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(color));
        }

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        masterTrack->render();
        colorMasterTrack.w = 0.2f;
        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(colorMasterTrack));

        for (auto i = 0; i < _composer->_tracks.size(); ++i) {
            Track* track = _composer->_tracks[i].get();
            ImGui::TableSetColumnIndex(i + 1);
            track->render();
            ImVec4 color = _selectedTracks.contains(track) ? selectedColor(track->_color) : track->_color;
            color.w = 0.2f;
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(color));
        }

        ImGui::EndTable();
    }

    if (ImGui::Button("Add track")) {
        _composer->addTrack();
    }

    ImGui::SameLine();
    ImGui::Text(_statusMessage.c_str());

    ImGui::End();

    if (_showAudioEngineWindow) {
        _audioEngineWindow->render();
    }

    if (_saveWindow != nullptr) {
        _saveWindow->render();
    }


    handleLocalShortcut();
    handleGlobalShortcut();
}

void ComposerWindow::setStatusMessage(std::string message) {
    _statusMessage = message;
}

void ComposerWindow::handleGlobalShortcut() {
    if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
        if (_composer->_playing) {
            _composer->stop();
        } else {
            _composer->play();
        }
        return;
    }

    auto& io = ImGui::GetIO();
    if (io.KeyCtrl) {
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Y))) {
            _composer->_commandManager.redo();
        } else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Z))) {
            _composer->_commandManager.undo();
        }
    }
}

void ComposerWindow::handleLocalShortcut() {
    // TODO 動かない。なぜ？
    //if (!canHandleInput()) {
    //    return;
    //}

    auto& io = ImGui::GetIO();
    if (io.KeyCtrl) {
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_C))) {
            // COPY
            if (!_selectedTracks.empty()) {
                nlohmann::json json;
                for (const auto& track : _selectedTracks) {
                    json["tracks"].push_back(track->toJson());
                }
                ImGui::SetClipboardText(eraseNekoId(json).dump(2).c_str());
            }
        } else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_X))) {
            // CUT
        } else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_V))) {
            // PAST
            nlohmann::json json = nlohmann::json::parse(ImGui::GetClipboardText());
            if (json.contains("tracks") && json["tracks"].is_array()) {
                for (const auto& x : json["tracks"]) {
                    Track* track = new Track(x);
                    _composer->addTrack(track);
                }
            }
        }
    }
}
