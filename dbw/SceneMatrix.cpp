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
        int ncolumns = static_cast<int>(_composer->_tracks.size() + 2);
        if (ImGui::BeginTable("Scene Matrix Table", ncolumns,
                              ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY,
                              ImVec2(-1.0f, -20.0f))) {
            ImGui::TableSetupScrollFreeze(1, 1);
            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_NoHide);
            for (auto& track : _composer->_tracks) {
                ImGui::TableSetupColumn(track->_name.c_str());
            }
            ImGui::TableSetupColumn(_composer->_masterTrack->_name.c_str());

            ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
            ImGui::TableSetColumnIndex(0);
            ImGui::TableHeader("");

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
                        if (ImGui::BeginDragDropTarget()) {
                            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Sequence Matrix Clip")) {
                                const Clip* clip = (Clip*)payload->Data;
                                lane->getClipSlot(track.get())->_clip.reset(new Clip(*clip));
                            }
                            ImGui::EndDragDropTarget();
                        }
                    }
                }
                ImGui::TableSetColumnIndex(++columnIndex);
                for (const auto& lane : scene->_lanes) {
                    lane->render(_composer->_masterTrack.get());
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
    double oneBeatSec = 60.0 / track->_composer->_bpm;
    double sampleRate = track->_composer->_audioEngine->_sampleRate;
    for (auto& scene : _scenes) {
        for (auto& lane : scene->_lanes) {
            auto& clipSlot = lane->getClipSlot(track);
            if (!clipSlot->_playing) {
                continue;
            }
            double sequenceDuration = clipSlot->_clip->_sequence->_duration;
            double begin = fmod(_composer->_playTime, sequenceDuration);
            double end = fmod(_composer->_nextPlayTime, sequenceDuration);
            for (auto& note : clipSlot->_clip->_sequence->_notes) {
                double time = note->_time;
                if ((begin <= time && time < end) || (end < begin && (begin <= time || time < end))) {
                    int16_t channel = 0;
                    uint32_t sampleOffsetDouble = 0;
                    if (begin < end) {
                        sampleOffsetDouble = (time - begin) * oneBeatSec * sampleRate;
                    } else {
                        sampleOffsetDouble = (time + sequenceDuration - begin) * oneBeatSec * sampleRate;
                    }
                    uint32_t sampleOffset = std::round(sampleOffsetDouble);
                    track->_processBuffer._eventIn.noteOn(note->_key, channel, note->_velocity, sampleOffset);
                }
                time = note->_time + note->_duration;
                if ((begin <= time && time < end) || (end < begin && (begin <= time || time < end))) {
                    int16_t channel = 0;
                    uint32_t sampleOffsetDouble = 0;
                    if (begin < end) {
                        sampleOffsetDouble = (time - begin) * oneBeatSec * sampleRate;
                    } else {
                        sampleOffsetDouble = (time + sequenceDuration - begin) * oneBeatSec * sampleRate;
                    }
                    uint32_t sampleOffset = std::round(sampleOffsetDouble);
                    track->_processBuffer._eventIn.noteOff(note->_key, channel, 1.0f, sampleOffset);
                }
            }
        }
    }
}

void SceneMatrix::stop() {
    for (auto& scene : _scenes) {
        for (auto& lane : scene->_lanes) {
            for (auto& x : lane->_clipSlots) {
                x.second->stop();
            }
        }
    }
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
