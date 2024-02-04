#include "Lane.h"
#include <imgui.h>
#include "Clip.h"
#include "ClipSlot.h"

Lane::Lane(Scene* scene) : _scene(scene) {
}

void Lane::render(Track* track) {
    ImGui::PushID(track);
    ImGui::PushID(this);
    auto clip = findClip(track);
    if (clip) {
        clip->renderInScene();
    } else {
        if (ImGui::Button("+")) {
            // TODO undo
            _clipSlots.emplace_back(new ClipSlot(track, this, new Clip()));
        }
    }
    ImGui::PopID();
    ImGui::PopID();
}

Clip* Lane::findClip(Track* track) {
    for (auto& clipSlot : _clipSlots) {
        if (clipSlot->_track == track) {
            return clipSlot->_clip;
        }
    }
    return nullptr;
}
