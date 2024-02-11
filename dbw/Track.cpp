#include "Track.h"
#include <mutex>
#include "imgui.h"
#include "AudioEngine.h"
#include "Command.h"
#include "Composer.h"
#include "Midi.h"
#include "Module.h"
#include "PluginModule.h"
#include "PluginHost.h"
#include "TrackLane.h"

Track::Track(std::string name, Composer* composer) : _name(name), _composer(composer) {
    _trackLanes.emplace_back(new TrackLane());
    _lastKeys.push_back(0);
}

Track::~Track() {
    if (_out != nullptr) {
        free(_out);
    }
}

void Track::process(int64_t steadyTime) {
    /*
    int toLine = to->_delay == 0 ? to->_line : to->_line + 1;
    for (int lineIndex = from->_line; lineIndex <= toLine && lineIndex < _lines.size(); ++lineIndex) {
        auto& line = _lines[lineIndex];
        for (int columnIndex = 0; columnIndex < _ncolumns; ++columnIndex) {
            auto& column = line->_columns[columnIndex];
            auto lastKey = _lastKeys[columnIndex];
            PlayPosition linePosition{ ._line = lineIndex, ._delay = static_cast<unsigned char>(column->_delay) };
            if (linePosition < *from || *to <= linePosition) {
                continue;
            }
            auto delay = linePosition.diffInDelay(*from);
            uint32_t sampleOffset = delay * _composer->_samplePerDelay;

            if (column->_note.empty()) {
                continue;
            }
            int16_t key = noteToNumber(column->_note);
            if (key == NOTE_NONE) {
                continue;
            }
            if (key == NOTE_OFF) {
                _processBuffer._eventIn.noteOff(lastKey, 0, 0x7f, sampleOffset);
                continue;
            }
            if (lastKey != NOTE_NONE) {
                _processBuffer._eventIn.noteOff(lastKey, 0, 0x7f, sampleOffset);
            }
            _processBuffer._eventIn.noteOn(key, 0, column->_velocity, sampleOffset);
            _lastKeys[columnIndex] = key;
        }
    }
    */

    _composer->_sceneMatrix->process(this);

    for (auto module = _modules.begin(); module != _modules.end(); ++module) {
        (*module)->process(&_processBuffer, steadyTime);
        _processBuffer.swapInOut();
    }
    _processBuffer.swapInOut();
}

void Track::render() {
    for (auto module = _modules.begin(); module != _modules.end(); ++module) {
        ImGui::PushID((*module).get());
        (*module)->render();
        ImGui::PopID();
    }
    if (ImGui::Button("+")) {
        _openModuleSelector = true;
    }
    if (_openModuleSelector) {
        _composer->_pluginManager.openModuleSelector(this);
    }
}

void Track::addModule(std::string path, uint32_t index) {
    PluginHost* pluginHost = new PluginHost(this);
    pluginHost->load(path.c_str(), index);
    Module* module = new PluginModule(pluginHost->_name, this, pluginHost);
    _composer->_commandManager.executeCommand(new AddModuleCommand(this, module));
}
