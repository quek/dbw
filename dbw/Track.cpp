#include "Track.h"
#include <mutex>
#include "imgui.h"
#include "AudioEngine.h"
#include "Column.h"
#include "Command.h"
#include "Composer.h"
#include "Line.h"
#include "Midi.h"
#include "Module.h"
#include "PluginModule.h"
#include "PluginHost.h"

Track::Track(std::string name, Composer* composer) : _name(name), _composer(composer), _ncolumns(1)
{
    _lastKeys.push_back(0);
    for (auto i = 0; i < composer->_maxLine; ++i) {
        _lines.push_back(std::make_unique<Line>(this, _ncolumns));
    }
    _lines[0x00].reset(new Line(Midi::C4, 0x64, 0, this));
    _lines[0x01].reset(new Line(Midi::C4, 0x64, 0x80, this));
    _lines[0x02].reset(new Line(Midi::C4, 0x40, 0x00, this));
    _lines[0x03].reset(new Line(Midi::C4, 0x7f, 0x00, this));
    _lines[0x04].reset(new Line(Midi::E4, 0x64, 0x00, this));
    _lines[0x06].reset(new Line(Midi::G4, 0x64, 0x00, this));
    _lines[0x07].reset(new Line(Midi::A4, 0x64, 0x00, this));
    _lines[0x08].reset(new Line(Midi::OFF, 0x64, 0x00, this));
    _lines[0x10].reset(new Line(Midi::E4, 0x64, 0x00, this));
    _lines[0x14].reset(new Line(Midi::G4, 0x64, 0x00, this));
    _lines[0x16].reset(new Line(Midi::A4, 0x64, 0x00, this));
    _lines[0x17].reset(new Line(Midi::B4, 0x64, 0x00, this));
    _lines[0x18].reset(new Line(Midi::OFF, 0x64, 0x00, this));
    _lines[0x20].reset(new Line(Midi::C4, 0x64, 0x00, this));
    _lines[0x21].reset(new Line(Midi::C4, 0x64, 0x00, this));
    _lines[0x22].reset(new Line(Midi::OFF, 0x64, 0x00, this));
    _lines[0x24].reset(new Line(Midi::E4, 0x64, 0x00, this));
    _lines[0x26].reset(new Line(Midi::G4, 0x64, 0x00, this));
    _lines[0x27].reset(new Line(Midi::A4, 0x64, 0x00, this));
    _lines[0x28].reset(new Line(Midi::OFF, 0x64, 0x00, this));
    _lines[0x30].reset(new Line(Midi::D4, 0x64, 0x00, this));
    _lines[0x34].reset(new Line(Midi::F4, 0x64, 0x00, this));
    _lines[0x36].reset(new Line(Midi::G4, 0x64, 0x00, this));
    _lines[0x37].reset(new Line(Midi::C5, 0x64, 0x00, this));
    _lines[0x38].reset(new Line(Midi::OFF, 0x64, 0x00, this));
    _lines[0x3a].reset(new Line(Midi::C5, 0x70, 0x00, this));
    _lines[0x3c].reset(new Line(Midi::C5, 0x74, 0x00, this));
    _lines[0x3d].reset(new Line(Midi::D5, 0x78, 0x00, this));
    _lines[0x3e].reset(new Line(Midi::B4, 0x7f, 0x00, this));
    _lines[0x3f].reset(new Line(Midi::OFF, 0x64, 0x00, this));

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
        _lines.push_back(std::make_unique<Line>(this, _ncolumns));
    }
}

void Track::process(const ProcessBuffer* in, unsigned long framesPerBuffer, int64_t steadyTime)
{
    _processBuffer.clear();
    _processBuffer.copyInToInFrom(in);

    PlayPosition* from = &_composer->_playPosition;
    PlayPosition* to = &_composer->_nextPlayPosition;
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

    ProcessBuffer* buffer = &_processBuffer;
    for (auto module = _modules.begin(); module != _modules.end(); ++module) {
        (*module)->process(buffer, framesPerBuffer, steadyTime);
        buffer = &(*module)->_processBuffer;
    }
    _processBuffer.copyOutToOutFrom(buffer);
}

void Track::render()
{
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
