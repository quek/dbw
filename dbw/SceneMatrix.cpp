#include "SceneMatrix.h"
#include <mutex>
#include <ranges>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "App.h"
#include "Clip.h"
#include "ClipSlot.h"
#include "Composer.h"
#include "Config.h"
#include "Lane.h"

SceneMatrix::SceneMatrix(const nlohmann::json& json) : Nameable(json) {
    for (const auto& x : json["_scenes"]) {
        Scene* scene = new Scene(x);
        scene->_sceneMatrix = this;
        _scenes.emplace_back(scene);
    }
}

SceneMatrix::SceneMatrix(Composer* composer) : _composer(composer) {
    addScene(false);
}

void SceneMatrix::render() {
    if (ImGui::Begin("Scene Matrix")) {
        int ncolumns = 1;
        for (const auto& track : _composer->allTracks()) {
            ncolumns += static_cast<int>(track->_lanes.size());
        }
        if (ImGui::BeginTable("Scene Matrix Table", ncolumns,
                              ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY,
                              ImVec2(-1.0f, -20.0f))) {
            ImGui::TableSetupScrollFreeze(1, 1);
            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_NoHide);
            ImGui::TableSetupColumn(_composer->_masterTrack->_name.c_str());
            for (auto& track : _composer->_masterTrack->getTracks()) {
                ImGui::TableSetupColumn(track->_name.c_str());
            }

            ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
            ImGui::TableSetColumnIndex(0);
            ImGui::TableHeader("");

            ImGui::TableSetColumnIndex(1);
            ImGui::PushID(_composer->_masterTrack.get());
            ImGui::TableHeader(_composer->_masterTrack->_name.c_str());
            ImGui::PopID();

            int columnIndex = 1;
            for (auto& track : _composer->_masterTrack->getTracks()) {
                ImGui::TableSetColumnIndex(++columnIndex);
                ImGui::PushID(track.get());
                auto name = track->_name.c_str();
                ImGui::TableHeader(name);
                ImGui::PopID();
            }

            for (const auto& scene : _scenes) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                ImGui::BeginDisabled(scene->isAllLanePlaying());
                if (ImGui::Button("▶")) {
                    scene->play();
                }
                ImGui::EndDisabled();
                ImGui::SameLine();
                ImGui::BeginDisabled(scene->isAllLaneStoped());
                if (ImGui::Button("■")) {
                    scene->stop();
                }
                ImGui::EndDisabled();
                ImGui::PopStyleVar();
                ImGui::SameLine();
                ImGui::Text(scene->_name.c_str());

                columnIndex = 0;
                ImGui::TableSetColumnIndex(++columnIndex);
                // TODO group tracks
                for (const auto& lane : _composer->_masterTrack->_lanes) {
                    auto& clipSlot = lane->getClipSlot(scene.get());
                    clipSlot->render(_composer);
                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Sequence Matrix Clip")) {
                            Clip* clip = (Clip*)payload->Data;
                            clipSlot->_clip.reset(clip->clone());
                        }
                        ImGui::EndDragDropTarget();
                    }
                }
                for (const auto& track : _composer->_masterTrack->getTracks()) {
                    for (const auto& lane : track->_lanes) {
                        ImGui::TableSetColumnIndex(++columnIndex);
                        auto& clipSlot = lane->getClipSlot(scene.get());
                        clipSlot->render(_composer);
                        if (ImGui::BeginDragDropTarget()) {
                            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Sequence Matrix Clip")) {
                                Clip* clip = (Clip*)payload->Data;
                                clipSlot->_clip.reset(clip->clone());
                            }
                            ImGui::EndDragDropTarget();
                        }
                    }
                }
            }
            ImGui::EndTable();
        }

        if (ImGui::Button("Add Scene##addScene")) {
            addScene();
        }
    }
    ImGui::End();
}

void SceneMatrix::process(Track* track) {
    double oneBeatSec = 60.0 / track->getComposer()->_bpm;
    for (auto& scene : _scenes) {
        for (auto& lane : track->_lanes) {
            auto& clipSlot = lane->getClipSlot(scene.get());
            if (!clipSlot->_playing) {
                continue;
            }
            if (!clipSlot->_clip) {
                continue;
            }
            double sequenceDuration = clipSlot->_clip->_sequence->_duration;
            double begin = fmod(_composer->_playTime, sequenceDuration);
            double end = fmod(_composer->_nextPlayTime, sequenceDuration);
            for (auto& note : clipSlot->_clip->_sequence->getItems()) {
                note->prepareProcessBuffer(&track->_processBuffer, begin, end, 0, sequenceDuration, oneBeatSec);
            }
        }
    }
}

void SceneMatrix::stop() {
    for (auto& scene : _scenes) {
        scene->stop();
    }
}

void SceneMatrix::addScene(bool undoable) {
    _composer->_commandManager.executeCommand(new AddSceneCommand(this, undoable));
}

nlohmann::json SceneMatrix::toJson() {
    nlohmann::json json = Nameable::toJson();

    nlohmann::json scenes = nlohmann::json::array();
    for (const auto& scene : _scenes) {
        scenes.emplace_back(scene->toJson());
    }
    json["_scenes"] = scenes;

    return json;
}

AddSceneCommand::AddSceneCommand(SceneMatrix* sceneMatrix, bool undoable) : _sceneMatrix(sceneMatrix), Command(undoable) {
}

void AddSceneCommand::execute(Composer* composer) {
    if (!_scene) {
        _scene = std::make_unique<Scene>(_sceneMatrix);
    }
    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
    _sceneMatrix->_scenes.push_back(std::move(_scene));
}

void AddSceneCommand::undo(Composer* composer) {
    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
    _scene = std::move(_sceneMatrix->_scenes.back());
    _sceneMatrix->_scenes.pop_back();
}
