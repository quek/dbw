#include "Track.h"
#include <mutex>
#include "imgui.h"
#include "AudioEngine.h"
#include "Command.h"
#include "Composer.h"
#include "Line.h"
#include "Midi.h"
#include "Module.h"
#include "PluginModule.h"
#include "PluginHost.h"

Track::Track(std::string name, Composer* composer) : _name(name), _composer(composer), _lastKey(NOTE_NONE)
{
    for (auto i = 0; i < composer->_maxLine; ++i) {
        _lines.push_back(std::make_unique<Line>(this));
    }
    //_lines[0x00]->_note.assign(Midi::C4);
    _lines[0x00].reset(new Line(Midi::C4, 0x64, 0, this));
    //_lines[0x01]->_note.assign(Midi::C4);
    //_lines[0x01]->_delay = 0x80;
    _lines[0x01].reset(new Line(Midi::C4, 0x64, 0x80, this));
    //_lines[0x02]->_note.assign(Midi::C4);
    //_lines[0x02]->_velocity = 0x40;
    _lines[0x02].reset(new Line(Midi::C4, 0x40, 0x00, this));

    _lines[0x03]->_note.assign(Midi::C4);
    _lines[0x03]->_velocity = 0x7f;
    _lines[0x04]->_note.assign(Midi::E4);
    _lines[0x06]->_note.assign(Midi::G4);
    _lines[0x07]->_note.assign(Midi::A4);
    _lines[0x08]->_note.assign(Midi::OFF);
    _lines[0x10]->_note.assign(Midi::E4);
    _lines[0x14]->_note.assign(Midi::G4);
    _lines[0x16]->_note.assign(Midi::A4);
    _lines[0x17]->_note.assign(Midi::B4);
    _lines[0x18]->_note.assign(Midi::OFF);
    _lines[0x20]->_note.assign(Midi::C4);
    _lines[0x21]->_note.assign(Midi::C4);
    _lines[0x22]->_note.assign(Midi::OFF);
    _lines[0x24]->_note.assign(Midi::E4);
    _lines[0x26]->_note.assign(Midi::G4);
    _lines[0x27]->_note.assign(Midi::A4);
    _lines[0x28]->_note.assign(Midi::OFF);
    _lines[0x30]->_note.assign(Midi::D4);
    _lines[0x34]->_note.assign(Midi::F4);
    _lines[0x36]->_note.assign(Midi::G4);
    _lines[0x37]->_note.assign(Midi::C5);
    _lines[0x38]->_note.assign(Midi::OFF);
    _lines[0x3a]->_note.assign(Midi::C5);
    _lines[0x3a]->_velocity = 0x70;
    _lines[0x3c]->_note.assign(Midi::C5);
    _lines[0x3c]->_velocity = 0x74;
    _lines[0x3d]->_note.assign(Midi::D5);
    _lines[0x3d]->_velocity = 0x78;
    _lines[0x3e]->_note.assign(Midi::B4);
    _lines[0x3e]->_velocity = 0x7f;
    _lines[0x3f]->_note.assign(Midi::OFF);

}

Track::~Track()
{
    if (_out != nullptr) {
        free(_out);
    }
}

void Track::changeMaxLine(int value)
{
    for (auto i = _lines.size(); i < value; ++i) {
        _lines.push_back(std::make_unique<Line>(this));
    }
}

void Track::process(const ProcessBuffer* in, unsigned long framesPerBuffer, int64_t steadyTime)
{
    _processBuffer.clear();
    _processBuffer.copyInToInFrom(in);

    PlayPosition* from = &_composer->_playPosition;
    PlayPosition* to = &_composer->_nextPlayPosition;
    int toLine = to->_delay == 0 ? to->_line : to->_line + 1;
    for (int i = from->_line; i <= toLine && i < _lines.size(); ++i) {
        PlayPosition linePosition{ ._line = i, ._delay = static_cast<unsigned char>(_lines[i]->_delay) };
        if (linePosition < *from || *to <= linePosition) {
            continue;
        }
        auto delay = linePosition.diffInDelay(*from);
        uint32_t sampleOffset = delay * _composer->_samplePerDelay;

        if (_lines[i]->_note.empty()) {
            continue;
        }
        int16_t key = noteToNumber(_lines[i]->_note);
        if (key == NOTE_NONE) {
            continue;
        }
        if (key == NOTE_OFF) {
            _processBuffer._eventIn.noteOff(_lastKey, 0, 0x7f, sampleOffset);
            continue;
        }
        if (_lastKey != NOTE_NONE) {
            _processBuffer._eventIn.noteOff(_lastKey, 0, 0x7f, sampleOffset);
        }
        _processBuffer._eventIn.noteOn(key, 0, _lines[i]->_velocity, sampleOffset);
        _lastKey = key;
    }

    ProcessBuffer* buffer = &_processBuffer;
    for (auto module = _modules.begin(); module != _modules.end(); ++module) {
        (*module)->process(buffer, framesPerBuffer, steadyTime);
        buffer = &(*module)->_processBuffer;
    }
    _processBuffer.copyOutToOutFrom(buffer);
}

void Track::render()
{
    ImGui::Text(_name.c_str());
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

void Track::renderLine(int line)
{
    _lines[line]->render();
}

class AddModuleCommand : public Command {
public:
    AddModuleCommand(Track* track, Module* module) : _track(track), _module(module) {}
    void execute(Composer* composer) override {
        std::lock_guard<std::mutex> lock(composer->_audioEngine->mtx);
        _track->_modules.push_back(std::move(_module));
        _track->_modules.back()->start();
        _track->_modules.back()->openGui();
    }
    void undo(Composer* composer) override {
        std::lock_guard<std::mutex> lock(composer->_audioEngine->mtx);
        _module = std::move(_track->_modules.back());
        _track->_modules.pop_back();
        _module->closeGui();
        _module->stop();
    }
    Track* _track;
    std::unique_ptr<Module> _module;
};

void Track::addModule(std::string path, uint32_t index) {
    PluginHost* pluginHost = new PluginHost();
    pluginHost->load(path.c_str(), index);
    Module* module = new PluginModule(pluginHost->_name, this, pluginHost);
    _composer->_commandManager.executeCommand(new AddModuleCommand(this, module));
}
