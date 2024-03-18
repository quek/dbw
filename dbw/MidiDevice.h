#pragma once
#include <string>
#include <memory>
#include <vector>
#include <rtmidi/RtMidi.h>
#include "PluginEventList.h"

class MidiDevice
{
public:
    MidiDevice(const std::string& name);
    virtual ~MidiDevice();
    void close();
    PluginEventList& getEventList() { return _eventList; }
    void read();

    static bool renderMidiInPorts();
    static void scan();
    static void showMidiInPortsWindow() { _show = true; }
private:
    PluginEventList _eventList;
    std::string _name;
    std::unique_ptr<RtMidiIn> _midiIn;

    static std::vector<std::tuple<unsigned int, std::string>> _midiInPorts;
    static bool _show;
};

