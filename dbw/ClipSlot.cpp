#include "ClipSlot.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "Composer.h"
#include "NoteClip.h"
#include "Track.h"

ClipSlot::ClipSlot() {
}

ClipSlot::ClipSlot(const nlohmann::json& json, SerializeContext& context) : Nameable(json, context) {
    if (json.contains("_clip")) {
        _clip.reset(Clip::create(json["_clip"], context));
    }
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
            _clip.reset(new NoteClip());
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

nlohmann::json ClipSlot::toJson(SerializeContext& context) {
    nlohmann::json json = Nameable::toJson(context);
    if (_clip) {
        json["_clip"] = _clip->toJson(context);
    }
    return json;
}
