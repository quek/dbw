#include "Lane.h"
#include <imgui.h>
#include "Clip.h"
#include "ClipSlot.h"

Lane::Lane(Scene* scene) : _scene(scene) {
}

void Lane::render(Track* track) {
    ImGui::PushID(track);
    ImGui::PushID(this);
    auto& clipSlot = getClipSlot(track);
    clipSlot->render();
    ImGui::PopID();
    ImGui::PopID();
}

std::unique_ptr<ClipSlot>& Lane::getClipSlot(Track* track) {
    auto x = _clipSlots.find(track);
    if (x != _clipSlots.end()) {
        return x->second;
    }
    _clipSlots[track] = std::make_unique<ClipSlot>(track, this);
    return _clipSlots[track];
}
