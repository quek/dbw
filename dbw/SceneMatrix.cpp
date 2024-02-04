#include "SceneMatrix.h"
#include <mutex>
#include <imgui.h>
#include "AudioEngine.h"
#include "Composer.h"

SceneMatrix::SceneMatrix(Composer* composer) : _composer(composer) {
    addScene(false);
}

void SceneMatrix::render() {
    if (ImGui::Begin("Scene Matrix")) {
        int ncolumns = _composer->_tracks.size() + 2;
        if (ImGui::BeginTable("Scene Matrix Table", ncolumns)) {
            ImGui::TableSetupScrollFreeze(1, 1);
            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoResize);
            for (auto& track : _composer->_tracks) {
                ImGui::TableSetupColumn(track->_name.c_str(), ImGuiTableColumnFlags_NoResize);
            }
            ImGui::TableSetupColumn(_composer->_masterTrack->_name.c_str(), ImGuiTableColumnFlags_NoResize);

            ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
            ImGui::TableSetColumnIndex(0);
            ImGui::TableHeader("#");

            int columnIndex = 0;
            for (auto& track : _composer->_tracks) {
                ImGui::TableSetColumnIndex(++columnIndex);
                ImGui::PushID(track.get());
                auto name = track->_name.c_str();
                ImGui::TableHeader(name);
                ImGui::PopID();
            }
            ImGui::TableSetColumnIndex(static_cast<int>(_composer->_tracks.size() + 1));
            ImGui::PushID(_composer->_masterTrack.get());
            ImGui::TableHeader(_composer->_masterTrack->_name.c_str());
            ImGui::PopID();

            for (const auto& scene : _scenes) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text(scene->_name.c_str());

                columnIndex = 0;
                for (const auto& track : _composer->_tracks) {
                    ImGui::TableSetColumnIndex(++columnIndex);
                    for (const auto& lane : scene->_lanes) {
                        lane->render(track.get());
                    }
                }
                ImGui::TableSetColumnIndex(++columnIndex);
                for (const auto& lane : scene->_lanes) {
                    lane->render(_composer->_masterTrack.get());
                }
            }
            ImGui::EndTable();
        }

        if (ImGui::Button("+##addScene")) {
            addScene();
        }
    }
    ImGui::End();
}

void SceneMatrix::addScene(bool undoable) {
    _composer->_commandManager.executeCommand(new AddSceneCommand(this, undoable));
}

AddSceneCommand::AddSceneCommand(SceneMatrix* sceneMatrix, bool undoable) : _sceneMatrix(sceneMatrix), Command(undoable) {
}

void AddSceneCommand::execute(Composer* composer) {
    if (!_scene) {
        _scene = std::make_unique<Scene>(_sceneMatrix);
    }
    std::lock_guard<std::mutex> lock(composer->_audioEngine->mtx);
    _sceneMatrix->_scenes.push_back(std::move(_scene));
}

void AddSceneCommand::undo(Composer* composer) {
    std::lock_guard<std::mutex> lock(composer->_audioEngine->mtx);
    _scene = std::move(_sceneMatrix->_scenes.back());
    _sceneMatrix->_scenes.pop_back();
}
