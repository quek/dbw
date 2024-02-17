#include "ClipSlot.h"
#include <imgui.h>
#include "Composer.h"
#include "Track.h"

ClipSlot::ClipSlot() {
}

void ClipSlot::render(Composer* composer) {
    ImGui::PushID(this);
    if (_clip) {
        if (_playing) {
            if (ImGui::Button("■")) {
                stop();
            }
        } else {
            if (ImGui::Button("▶")) {
                play();
                composer->play();
            }
        }
        ImGui::SameLine();
        _clip->renderInScene(composer->_pianoRollWindow.get());
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
        if (ImGui::Button("+", ImVec2(-FLT_MIN, 0.0f))) {
            // TODO undo
            _clip = std::make_unique<Clip>();
        }
        ImGui::PopStyleColor();
    }
    ImGui::PopID();
}

void ClipSlot::play() {
    if (_playing) {
        return;
    }
    _playing = true;
}

void ClipSlot::stop() {
    if (!_playing) {
        return;
    }
    _playing = false;
}
