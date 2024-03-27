#include "SidechainInputSelector.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "Composer.h"
#include "Module.h"
#include "Track.h"

SidechainInputSelector::SidechainInputSelector(Composer* composer) : _composer(composer) {
}

void SidechainInputSelector::open(Module* module, int inputIndex) {
    _module = module;
    _inputIndex = inputIndex;
    _show = true;
}

void SidechainInputSelector::render() {
    if (!_show) {
        return;
    }
    ImGui::OpenPopup(NAME);
    if (ImGui::BeginPopupModal(NAME, &_show)) {
        render(_composer->_masterTrack.get());
        ImGui::EndPopup();
    }
}

void SidechainInputSelector::render(Track* track) {
    if (!track->isAvailableSidechainSrc(_module->_track)) {
        return;
    }
    if (ImGui::TreeNode(track->_name.c_str())) {
        if (track != _module->_track) {
            if (ImGui::Button("PRE")) {
                _module->connectPre(track->_fader.get(), 0, _inputIndex);
                _show = false;
                _composer->computeProcessOrder();
            }
            ImGui::SameLine();
            if (ImGui::Button("POST")) {
                _module->connect((Module*)track->_fader.get(), 0, _inputIndex);
                _show = false;
                _composer->computeProcessOrder();
            }
        }
        for (auto& module : track->_modules) {
            if (module.get() == _module) {
                break;
            }
            if (ImGui::Button(module->_name.c_str())) {
                // TODO outputIndex
                _module->connect(module.get(), 0, _inputIndex);
                _show = false;
                _composer->computeProcessOrder();
            }
        }
        ImGui::TreePop();
    }

    for (const auto& x : track->getTracks()) {
        render(x.get());
    }
}
