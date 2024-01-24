#include "Composer.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "logging.h"
#include <sstream>
#include <cstdlib>
#include <map>

ImVec4 COLOR_PALY_LINE = ImVec4(0.3f, 0.7f, 0.7f, 0.65f);
ImVec4 COLOR_BUTTON_ON = ImVec4(0.3f, 0.7f, 0.7f, 0.65f);
ImVec4 COLOR_BUTTON_ON_HOVERED = ImVec4(0.5f, 0.9f, 0.9f, 0.65f);
ImVec4 COLOR_BUTTON_ON_ACTIVE = ImVec4(0.4f, 0.8f, 0.8f, 0.65f);

Composer::Composer(AudioEngine* audioEngine) : _audioEngine(audioEngine)
{
    addTrack();
}

void Composer::process(float* in, float* out, unsigned long framesPerBuffer, int64_t steadyTime) {
    if (_playing) {
        _nextPlayPosition = _playPosition.nextPlayPosition(_audioEngine->_sampleRate, framesPerBuffer, _bpm, _lpb);
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
    ImGui::DragFloat("BPM", &_bpm, 1.0f, 0.0f, 999.0f, "%.02f");

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
        ImGuiIO& io = ImGui::GetIO();
        if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
            _playing = !_playing;
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

void Composer::addTrack()
{
    std::stringstream name;
    name << "track " << _tracks.size() + 1;
    {
        std::lock_guard<std::mutex> lock(_audioEngine->mtx);
        _tracks.push_back(std::make_unique<Track>(name.str(), this));
    }
}

Track::Track(std::string name, Composer* composer) : _name(name), _composer(composer)
{
    for (auto i = 0; i < composer->_maxLine; ++i) {
        _lines.push_back(std::make_unique<std::string>());
    }
    _lines[0x00]->assign(Midi::C4);
    _lines[0x04]->assign(Midi::E4);
    _lines[0x08]->assign(Midi::OFF);
    _lines[0x0a]->assign(Midi::G4);
    _lines[0x0f]->assign(Midi::OFF);
    _lines[0x10]->assign(Midi::C4);
    _lines[0x14]->assign(Midi::E4);
    _lines[0x18]->assign(Midi::OFF);
    _lines[0x1a]->assign(Midi::G4);
    _lines[0x1f]->assign(Midi::OFF);
    _lines[0x20]->assign(Midi::C4);
    _lines[0x24]->assign(Midi::E4);
    _lines[0x28]->assign(Midi::OFF);
    _lines[0x2a]->assign(Midi::G4);
    _lines[0x2f]->assign(Midi::OFF);
    _lines[0x30]->assign(Midi::C4);
    _lines[0x34]->assign(Midi::E4);
    _lines[0x38]->assign(Midi::OFF);
    _lines[0x3a]->assign(Midi::G4);
    _lines[0x3f]->assign(Midi::OFF);
}

Track::~Track()
{
    if (_out != nullptr) {
        free(_out);
    }
}

void Track::process(const AudioBuffer* in, unsigned long framesPerBuffer, int64_t steadyTime)
{
    _audioBuffer.clear();
    _audioBuffer.copyInToInFrom(in);

    PlayPosition* from = &_composer->_playPosition;
    PlayPosition* to = &_composer->_nextPlayPosition;
    int fromLine = from->_delay == 0 ? from->_line : from->_line + 1;
    int toLine = to->_delay == 0 ? to->_line : to->_line + 1;
    for (int i = fromLine; i < toLine && i < _lines.size(); ++i) {
        if (_lines[i]->empty()) {
            continue;
        }
        int16_t key = noteToNumber(*_lines[i]);
        if (key == NOTE_NONE) {
            continue;
        }
        if (key == NOTE_OFF) {
            _audioBuffer._eventIn.noteOff(_lastKey, 0, 100, 0);
            continue;
        }
        if (_lastKey != NOTE_NONE) {
            _audioBuffer._eventIn.noteOff(_lastKey, 0, 100, 0);
        }
        _audioBuffer._eventIn.noteOn(key, 0, 100, 0);
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
        (*i)->render();
    }
}

void Track::renderLine(int line)
{
    auto p = _lines[line].get();
    ImGui::PushID(p);
    if (ImGui::InputText("", p)) {
        std::transform(p->begin(), p->end(), p->begin(),
            [](auto c) { return static_cast<char>(std::toupper(c)); });
    }
    ImGui::PopID();
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

