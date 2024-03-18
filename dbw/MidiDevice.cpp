#include "MidiDevice.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "Config.h"
#include "Error.h"
#include "GuiUtil.h"
#include "PluginEventList.h"

constexpr const char* MIDI_IN_PORTS_POPUP = "MIDI IN PORTS";

std::vector<std::tuple<unsigned int, std::string>> MidiDevice::_midiInPorts;
bool MidiDevice::_show = false;

MidiDevice::MidiDevice(const std::string& name) : _name(name)
{

    _midiIn.reset(new RtMidiIn());
    unsigned int nPorts = _midiIn->getPortCount();
    for (unsigned int i = 0; i < nPorts; i++)
    {
        if (_midiIn->getPortName(i) == _name)
        {
            _midiIn->openPort(i);
            break;
        }
    }
}

MidiDevice::~MidiDevice()
{
    close();
}

void MidiDevice::close()
{
    _midiIn->closePort();
}

void MidiDevice::read()
{
    _eventList.clear();
    std::vector<unsigned char> message;
    for (;;)
    {
        _midiIn->getMessage(&message);
        if (message.empty()) return;
        if (message.size() < 3) continue;

        int16_t event_channel = message[0];
        int16_t channel = event_channel & 0x0f;
        int16_t event = event_channel & 0xf0;
        switch (event)
        {
        case 0x90:
        {
            int16_t key = message[1];
            int16_t velocity = message[2];
            _eventList.noteOn(key, channel, velocity / 127.0, 0);
            break;
        }
        case 0x80:
        {
            int16_t key = message[1];
            int16_t velocity = message[2];
            _eventList.noteOff(key, channel, velocity / 127.0, 0);
            break;
        }
        }
        message.clear();
    }
}

bool MidiDevice::renderMidiInPorts()
{
    bool result = false;
    if (_show)
    {
        ImGui::OpenPopup(MIDI_IN_PORTS_POPUP);
    }
    if (ImGui::BeginPopupModal(MIDI_IN_PORTS_POPUP, &_show, ImGuiWindowFlags_AlwaysAutoResize))
    {
        for (auto& [i, name] : _midiInPorts)
        {
            bool enabled = gPreference.midiInDevices.contains(name);
            if (ImGui::Checkbox(name.c_str(), &enabled))
            {
                if (enabled)
                {
                    gPreference.midiInDevices.insert(name);
                }
                else
                {
                    gPreference.midiInDevices.erase(name);
                }
                gPreference.save();
                result = true;
            }
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

    return result;
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
            _midiInPorts.push_back({ i, midiin->getPortName(i) });
        }
    }
    catch (RtMidiError& error)
    {
        Error("{}", error.what());
    }
}
