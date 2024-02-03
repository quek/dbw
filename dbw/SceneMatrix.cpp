#include "SceneMatrix.h"
#include <imgui.h>
#include "Composer.h"

SceneMatrix::SceneMatrix(Composer* composer) : _composer(composer) {
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

            int i = 1;
            for (auto& track : _composer->_tracks) {
                ImGui::TableSetColumnIndex(i);
                ImGui::PushID(track.get());
                auto name = track->_name.c_str();
                ImGui::TableHeader(name);
                ImGui::PopID();
                ++i;
            }
            ImGui::TableSetColumnIndex(static_cast<int>(_composer->_tracks.size() + 1));
            ImGui::PushID(_composer->_masterTrack.get());
            ImGui::TableHeader(_composer->_masterTrack->_name.c_str());
            ImGui::PopID();
            ImGui::EndTable();
        }
    }
    ImGui::End();
}
