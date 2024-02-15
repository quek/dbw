#include "ClipSlot.h"
#include <imgui.h>
#include "Composer.h"
#include "Track.h"

ClipSlot::ClipSlot(Track* track, Lane* lane, Clip* clip) : _track(track), _lane(lane), _clip(clip) {
}

void ClipSlot::render() {
    ImGui::PushID(this);
    if (_clip) {
        if (_playing) {
            if (ImGui::Button("■")) {
                stop();
            }
        } else {
            if (ImGui::Button("▶")) {
                play();
            }
        }
        ImGui::SameLine();
        _clip->renderInScene(_track->_composer->_pianoRollWindow.get());
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
    _track->_composer->play();
}

void ClipSlot::stop() {
    if (!_playing) {
        return;
    }
    _playing = false;
}
