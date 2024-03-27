#include "SidechainInputSelector.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "Composer.h"
#include "Module.h"
#include "Track.h"
#include "command/ModuleConnect.h"

SidechainInputSelector::SidechainInputSelector(Composer* composer) : _composer(composer)
{
}

void SidechainInputSelector::open(Module* module, int inputIndex)
{
    _module = module;
    _inputIndex = inputIndex;
    _show = true;
}

void SidechainInputSelector::render()
{
    if (!_show)
    {
        return;
    }
    ImGui::OpenPopup(NAME);
    if (ImGui::BeginPopupModal(NAME, &_show))
    {
        render(_composer->_masterTrack.get());
        ImGui::EndPopup();
    }
}

void SidechainInputSelector::render(Track* track)
{
    if (!track->isAvailableSidechainSrc(_module->_track))
    {
        return;
    }
    if (ImGui::TreeNode(track->_name.c_str()))
    {
        if (track != _module->_track)
        {
            if (ImGui::Button("PRE"))
            {
                _composer->commandExecute(new command::ModuleConnect((Module*)track->_fader.get(), 0, _module, _inputIndex, false));
                _show = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("POST"))
            {
                _composer->commandExecute(new command::ModuleConnect((Module*)track->_fader.get(), 0, _module, _inputIndex, true));
                _show = false;
            }
        }
        for (auto& module : track->_modules)
        {
            if (module.get() == _module)
            {
                break;
            }
            if (ImGui::Button(module->_name.c_str()))
            {
                // TODO outputIndex
                _composer->commandExecute(new command::ModuleConnect(module.get(), 0, _module, _inputIndex, true));
                _show = false;
            }
        }
        ImGui::TreePop();
    }

    for (const auto& x : track->getTracks())
    {
        render(x.get());
    }
}
