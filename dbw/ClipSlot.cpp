#include "ClipSlot.h"
#include <imgui.h>
#include "Composer.h"
#include "Track.h"

ClipSlot::ClipSlot(Track* track, Lane* lane, Clip* clip) : _track(track), _lane(lane), _clip(clip) {
}

void ClipSlot::render() {
    if (_clip) {
        ImGui::PushID(this);
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
        _clip->renderInScene(_track->_composer->_pianoRoll.get());
        ImGui::PopID();
    } else {
        if (ImGui::Button("+")) {
            // TODO undo
            _clip = std::make_unique<Clip>();
        }
    }
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
