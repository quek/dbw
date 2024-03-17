#include "MidiDevice.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <rtmidi/RtMidi.h>
#include "Error.h"
#include "GuiUtil.h"

constexpr const char* MIDI_IN_PORTS_POPUP = "MIDI IN PORTS";

std::vector<std::tuple<unsigned int, std::string, bool>> MidiDevice::_midiInPorts;
bool MidiDevice::_show = false;

void MidiDevice::renderMidiInPorts()
{
    if (_show)
    {
        ImGui::OpenPopup(MIDI_IN_PORTS_POPUP);
    }
    if (ImGui::BeginPopupModal(MIDI_IN_PORTS_POPUP, &_show, ImGuiWindowFlags_AlwaysAutoResize))
    {
        for (auto& [i, name, enabled] : _midiInPorts)
        {
            ImGui::Checkbox(name.c_str(), &enabled);
        }
        ImGui::Separator();
        if (ImGui::Button("Close"))
        {
            _show = false;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            _show = false;
        }
        ImGui::EndPopup();
    }
}

void MidiDevice::scan()
{
    std::unique_ptr<RtMidiIn> midiin;
    try
    {
        midiin.reset(new RtMidiIn());

        unsigned int nPorts = midiin->getPortCount();
        _midiInPorts.clear();
        for (unsigned int i = 0; i < nPorts; i++)
        {
            _midiInPorts.push_back({ i,midiin->getPortName(i), false });
        }
    }
    catch (RtMidiError& error)
    {
        Error("{}", error.what());
    }
}
