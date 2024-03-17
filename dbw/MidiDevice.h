#pragma once
#include <string>
#include <vector>

class MidiDevice
{
public:
    std::vector<std::pair<unsigned int, std::string>>& getMidiInPorts();
    static void renderMidiInPorts();
    static void scan();
    static void showMidiInPortsWindow() { _show = true; }
private:
    static std::vector<std::tuple<unsigned int, std::string, bool>> _midiInPorts;
    static bool _show;
};

