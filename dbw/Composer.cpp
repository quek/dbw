#include "Composer.h"
#include "Line.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "logging.h"
#include <sstream>
#include <cstdlib>
#include <map>
#include "GuiUtil.h"
#include "Command.h"

Composer::Composer(AudioEngine* audioEngine) : _audioEngine(audioEngine), _commandManager(this)
{
    addTrack();
}

void Composer::process(float* in, float* out, unsigned long framesPerBuffer, int64_t steadyTime) {
    if (_playing) {
        _nextPlayPosition = _playPosition.nextPlayPosition(_audioEngine->_sampleRate, framesPerBuffer, _bpm, _lpb, &_samplePerDelay);
    }

    for (unsigned long i = 0; i < framesPerBuffer * 2; ++i) {
        out[i] = 0.0;
    }
    AudioBuffer* audioBuffer = &_audioBuffer;
    audioBuffer->clear();
    audioBuffer->ensure(framesPerBuffer);
    for (unsigned long i = 0; i < framesPerBuffer * 2; ++i) {
        audioBuffer->_out[i] = in[i];
    }
    for (auto iter = _tracks.begin(); iter != _tracks.end(); ++iter) {
        auto& track = *iter;
        track->process(audioBuffer, framesPerBuffer, steadyTime);
        float* buffer = track->_audioBuffer._out.get();
        // TODO master track
        for (unsigned long i = 0; i < framesPerBuffer * 2; ++i) {
            out[i] += buffer[i];
        }
    }

    if (_playing) {
        _playPosition = _nextPlayPosition;
    }
    if (_looping) {
        if (_playPosition >= _loopEndPosition) {
            _playPosition = _loopStartPosition;
        }
    }
    if (_playPosition._line > _maxLine) {
        _playPosition._line = 0;
        _playPosition._delay = 0;
    }
}

void Composer::scanPlugin()
{
    _pluginManager.scan();
}

void Composer::changeMaxLine()
{
    for (auto track = _tracks.begin(); track != _tracks.end(); ++track) {
        (*track)->changeMaxLine(_maxLine);
    }
}

void Composer::render()
{
    ImGui::Begin("main window");   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)

    if (_playing) {
        ImGui::PushStyleColor(ImGuiCol_Button, COLOR_BUTTON_ON);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, COLOR_BUTTON_ON_HOVERED);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, COLOR_BUTTON_ON_ACTIVE);
        if (ImGui::Button("Play")) {
            stop();
        }
        ImGui::PopStyleColor(3);
    }
    else {
        if (ImGui::Button("Play")) {
            play();
        }
    }
    ImGui::SameLine();
    if (_looping) {
        ImGui::PushStyleColor(ImGuiCol_Button, COLOR_BUTTON_ON);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, COLOR_BUTTON_ON_HOVERED);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, COLOR_BUTTON_ON_ACTIVE);
        if (ImGui::Button("Loop")) {
            _looping = false;
        }
        ImGui::PopStyleColor(3);
    }
    else {
        if (ImGui::Button("Loop")) {
            _looping = true;
        }
    }
    ImGui::SameLine();
    if (_scrollLock) {
        ImGui::PushStyleColor(ImGuiCol_Button, COLOR_BUTTON_ON);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, COLOR_BUTTON_ON_HOVERED);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, COLOR_BUTTON_ON_ACTIVE);
        if (ImGui::Button("ScLk")) {
            _scrollLock = false;
        }
        ImGui::PopStyleColor(3);
    }
    else {
        if (ImGui::Button("ScLk")) {
            _scrollLock = true;
        }
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 4);
    if (ImGui::DragInt("Lines", &_maxLine, 1.0f, 1, 0x40 * 1000)) {
        changeMaxLine();
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 5);
    ImGui::DragFloat("BPM", &_bpm, 1.0f, 0.0f, 999.0f, "%.02f");
    ImGui::SameLine();
    if (ImGui::Button("Undo")) {
        _commandManager.undo();
    }
    ImGui::SameLine();
    if (ImGui::Button("Redo")) {
        _commandManager.redo();
    }
    ImGui::SameLine();
    if (ImGui::Button("Scan Plugin")) {
        scanPlugin();
    }

    ImGuiTableFlags flags = ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
    // When using ScrollX or ScrollY we need to specify a size for our table container!
    // Otherwise by default the table will fit all available space, like a BeginChild() call.
    ImVec2 outer_size = ImVec2(0.0f, TEXT_BASE_HEIGHT * 20);
    if (ImGui::BeginTable("tracks", 1 + static_cast<int>(_tracks.size()), flags, outer_size)) {
        ImGui::TableSetupScrollFreeze(1, 1);
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_NoHide); // Make the first column not hideable to match our use of TableSetupScrollFreeze()
        for (auto i = 0; i < _tracks.size(); ++i) {
            ImGui::TableSetupColumn(_tracks[i]->_name.c_str());
        }
        ImGui::TableHeadersRow();
        for (int line = 0; line < _maxLine; line++) {
            ImGui::TableNextRow();
            if (line == _playPosition._line) {
                ImU32 cell_bg_color = ImGui::GetColorU32(COLOR_PALY_LINE);
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, cell_bg_color);
                if (!_scrollLock) {
                    ImGui::SetScrollHereY(0.5f); // 0.0f:top, 0.5f:center, 1.0f:bottom
                }
            }
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%02X", line);
            for (auto i = 0; i < _tracks.size(); ++i) {
                auto track = _tracks[i].get();
                ImGui::TableSetColumnIndex(i + 1);
                track->renderLine(line);
            }
        }
        ImGui::EndTable();
    }


    for (auto i = 0; i < _tracks.size(); ++i) {
        ImGui::PushID(i);
        _tracks[i]->render();
        ImGui::PopID();
    }

    if (ImGui::Button("Add track")) {
        addTrack();
    }


    {
        if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
            if (_playing) {
                stop();
            }
            else {
                play();
            }
        }
    }

    ImGui::End();
}

void Composer::play()
{
    _playing = true;
}

void Composer::stop()
{
    _playing = false;
    _playPosition = PlayPosition{ ._line = 0, ._delay = 0 };
    _nextPlayPosition = PlayPosition{ ._line = 0, ._delay = 0 };
}

class AddTrackCommand : public Command {
public:
    AddTrackCommand(Track* track) : _track(track) {}
    void execute(Composer* composer) override {
        std::lock_guard<std::mutex> lock(composer->_audioEngine->mtx);
        composer->_tracks.push_back(std::move(_track));
    }
    void undo(Composer* composer) override {
        std::lock_guard<std::mutex> lock(composer->_audioEngine->mtx);
        _track = std::move(composer->_tracks.back());
        composer->_tracks.pop_back();
    }

    std::unique_ptr<Track> _track;

};

void Composer::addTrack()
{
    std::stringstream name;
    name << "track " << this->_tracks.size() + 1;
    Track* track = new Track(name.str(), this);
    _commandManager.executeCommand(new AddTrackCommand(track));
}

Track::Track(std::string name, Composer* composer) : _name(name), _composer(composer)
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

void Track::process(const AudioBuffer* in, unsigned long framesPerBuffer, int64_t steadyTime)
{
    _audioBuffer.clear();
    _audioBuffer.copyInToInFrom(in);

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
            _audioBuffer._eventIn.noteOff(_lastKey, 0, 0x7f, sampleOffset);
            continue;
        }
        if (_lastKey != NOTE_NONE) {
            _audioBuffer._eventIn.noteOff(_lastKey, 0, 0x7f, sampleOffset);
        }
        _audioBuffer._eventIn.noteOn(key, 0, _lines[i]->_velocity, sampleOffset);
        _lastKey = key;
    }

    AudioBuffer* buffer = &_audioBuffer;
    for (auto i = _modules.begin(); i != _modules.end(); ++i) {
        (*i)->process(buffer, framesPerBuffer, steadyTime);
        buffer = &(*i)->_audioBuffer;
    }
    _audioBuffer.copyOutToOutFrom(buffer);
}

void Track::render()
{
    ImGui::Text(_name.c_str());

    if (ImGui::Button("+")) {
        _openModuleSelector = true;
    }
    if (_openModuleSelector) {
        _composer->_pluginManager.openModuleSelector(this);
    }
    /*
    ImGui::InputText("plugin path", &_pluginPath);

    if (ImGui::Button("Load")) {
        PluginHost* pluginHost = new PluginHost();
        pluginHost->load(_pluginPath.c_str(), 0);
        pluginHost->start(_composer->_audioEngine->_sampleRate, _composer->_audioEngine->_bufferSize);
        pluginHost->openGui();
        std::lock_guard<std::mutex> lock(_composer->_audioEngine->mtx);
        _modules.push_back(std::make_unique<PluginModule>(pluginHost));
    }
    for (auto i = _modules.begin(); i != _modules.end(); ++i) {
        ImGui::PushID((*i).get());
        (*i)->render();
        ImGui::PopID();
    }
    */
}

void Track::renderLine(int line)
{
    _lines[line]->render();
}

PluginModule::PluginModule(PluginHost* pluginHost) : _pluginHost(pluginHost)
{
}

PluginModule::~PluginModule()
{
    _pluginHost->stop();
    _pluginHost->unload();
}

void PluginModule::process(AudioBuffer* in, unsigned long framesPerBuffer, int64_t steadyTime)
{
    Module::process(in, framesPerBuffer, steadyTime);
    auto process = _pluginHost->process(in, framesPerBuffer, steadyTime);

    auto buffer = process->audio_outputs;
    bool isLeftConstant = (buffer->constant_mask & (static_cast<uint64_t>(1) << 0)) != 0;
    bool isRightConstant = (buffer->constant_mask & (static_cast<uint64_t>(1) << 1)) != 0;
    float* out = _audioBuffer._out.get();
    for (unsigned long i = 0; i < framesPerBuffer; ++i) {
        *out = buffer->data32[0][isLeftConstant ? 0 : i];
        ++out;
        *out = buffer->data32[1][isRightConstant ? 0 : i];
        ++out;
    }
}

void PluginModule::render()
{
    if (ImGui::Button("Open")) {
        _pluginHost->openGui();
    }
    if (ImGui::Button("Close")) {
        _pluginHost->closeGui();
    }
}

AudioBuffer::AudioBuffer() : AudioBuffer(0)
{
}

AudioBuffer::AudioBuffer(unsigned long framesPerBuffer) :
    _in(std::make_unique<float[]>(framesPerBuffer * 2)),
    _out(std::make_unique<float[]>(framesPerBuffer * 2)),
    _framesPerBuffer(framesPerBuffer)
{
}

void AudioBuffer::ensure(unsigned long framesPerBuffer)
{
    if (_framesPerBuffer != framesPerBuffer) {
        _framesPerBuffer = framesPerBuffer;
        _in = std::make_unique<float[]>(static_cast<size_t>(framesPerBuffer) * 2);
        _out = std::make_unique<float[]>(static_cast<size_t>(framesPerBuffer) * 2);
    }
}

void AudioBuffer::copyInToInFrom(const AudioBuffer* from)
{
    if (this == from) {
        return;
    }
    ensure(from->_framesPerBuffer);
    memcpy(_in.get(), from->_in.get(), sizeof(float) * 2 * from->_framesPerBuffer);
}

void AudioBuffer::copyOutToOutFrom(const AudioBuffer* from)
{
    if (this == from) {
        return;
    }
    ensure(from->_framesPerBuffer);
    memcpy(_out.get(), from->_out.get(), sizeof(float) * 2 * from->_framesPerBuffer);
}

void AudioBuffer::clear()
{
    _eventIn.clear();
    _eventOut.clear();
}

void Module::render()
{
}

void Module::process(AudioBuffer* /* in */, unsigned long framesPerBuffer, int64_t /* steadyTime */)
{
    _audioBuffer.ensure(framesPerBuffer);
}

