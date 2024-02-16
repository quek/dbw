#include "SidechainInputSelector.h"
#include <imgui.h>
#include "Composer.h"
#include "Module.h"
#include "Track.h"

SidechainInputSelector::SidechainInputSelector(Composer* composer) : _composer(composer) {
}

void SidechainInputSelector::open(Module* module) {
    _module = module;
    _show = true;
}

void SidechainInputSelector::render() {
    if (!_module || !_show) {
        return;
    }
    ImGui::OpenPopup(NAME);
    if (ImGui::BeginPopupModal(NAME, &_show)) {
        for (auto& track : _composer->_tracks) {
            if (!track->isAvailableSidechainSrc(_module->_track)) {
                continue;
            }
            if (ImGui::TreeNode(track->_name.c_str())) {
                if (track.get() != _module->_track) {
                    if (ImGui::Button("PRE")) {
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("POST")) {
                    }
                }
                for (auto& module : track->_modules) {
                    if (module.get() == _module) {
                        break;
                    }
                    if (ImGui::Button(module->_name.c_str())) {

                    }
                }
                ImGui::TreePop();
            }
        }
        ImGui::EndPopup();
    }
}
