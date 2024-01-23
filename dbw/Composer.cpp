#include "Composer.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "PluginHost.h"
#include "logging.h"
#include <sstream>
#include <cstdlib>
#include <map>


const int16_t NOTE_OFF = 128;
const int16_t NOTE_NONE = -1;

int16_t noteToNumber(std::string note_name) {
    std::map<std::string, int> table = {
        {"OFF", NOTE_OFF},
        {"C0", 12}, {"C0#", 13}, {"D0", 14}, {"D0#", 15}, {"E0", 16},
        {"F0", 17}, {"F0#", 18}, {"G0", 19}, {"G0#", 20}, {"A0", 21},
        {"A0#", 22}, {"B0", 23}, {"C1", 24}, {"C1#", 25}, {"D1", 26},
        {"D1#", 27}, {"E1", 28}, {"F1", 29}, {"F1#", 30}, {"G1", 31},
        {"G1#", 32}, {"A1", 33}, {"A1#", 34}, {"B1", 35}, {"C2", 36},
        {"C2#", 37}, {"D2", 38}, {"D2#", 39}, {"E2", 40}, {"F2", 41},
        {"F2#", 42}, {"G2", 43}, {"G2#", 44}, {"A2", 45}, {"A2#", 46},
        {"B2", 47}, {"C3", 48}, {"C3#", 49}, {"D3", 50}, {"D3#", 51},
        {"E3", 52}, {"F3", 53}, {"F3#", 54}, {"G3", 55}, {"G3#", 56},
        {"A3", 57}, {"A3#", 58}, {"B3", 59}, {"C4", 60}, {"C4#", 61},
        {"D4", 62}, {"D4#", 63}, {"E4", 64}, {"F4", 65}, {"F4#", 66},
        {"G4", 67}, {"G4#", 68}, {"A4", 69}, {"A4#", 70}, {"B4", 71},
        {"C5", 72}, {"C5#", 73}, {"D5", 74}, {"D5#", 75}, {"E5", 76},
        {"F5", 77}, {"F5#", 78}, {"G5", 79}, {"G5#", 80}, {"A5", 81},
        {"A5#", 82}, {"B5", 83}, {"C6", 84}, {"C6#", 85}, {"D6", 86},
        {"D6#", 87}, {"E6", 88}, {"F6", 89}, {"F6#", 90}, {"G6", 91},
        {"G6#", 92}, {"A6", 93}, {"A6#", 94}, {"B6", 95}, {"C7", 96},
        {"C7#", 97}, {"D7", 98}, {"D7#", 99}, {"E7", 100}, {"F7", 101},
        {"F7#", 102}, {"G7", 103}, {"G7#", 104}, {"A7", 105}, {"A7#", 106},
        {"B7", 107}, {"C8", 108}, {"C8#", 109}, {"D8", 110}, {"D8#", 111},
        {"E8", 112}, {"F8", 113}, {"F8#", 114}, {"G8", 115}, {"G8#", 116},
        {"A8", 117}, {"A8#", 118}, {"B8", 119}, {"C9", 120}
    };

    auto x = table.find(note_name);
    if (x == table.end()) {
        return NOTE_NONE;
    }
    return static_cast<int16_t>(x->second);
}


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
        audioBuffer = &track->_audioBuffer;
        float* buffer = audioBuffer->_out.get();
        // TODO master track
        for (unsigned long i = 0; i < framesPerBuffer * 2; ++i) {
            out[i] += buffer[i];
        }
    }

    if (_playing) {
        _playPosition = _nextPlayPosition;
    }
}

void Composer::render()
{
    ImGui::Begin("main window");   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)

    if (_playing) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.7f, 0.7f, 0.65f));
        if (ImGui::Button("Play")) {
            stop();
        }
        ImGui::PopStyleColor();
    }
    else {
        if (ImGui::Button("Play")) {
            play();
        }
    }
    ImGui::SameLine();
    ImGui::DragFloat("BPM", &_bpm, 1.0f, 0.0f, 999.0f, "%.02f");

    ImGuiTableFlags flags = ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
    // When using ScrollX or ScrollY we need to specify a size for our table container!
    // Otherwise by default the table will fit all available space, like a BeginChild() call.
    ImVec2 outer_size = ImVec2(0.0f, TEXT_BASE_HEIGHT * 8);
    if (ImGui::BeginTable("tracks", 1 + static_cast<int>(_tracks.size()), flags, outer_size)) {
        ImGui::TableSetupScrollFreeze(1, 1);
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_NoHide); // Make the first column not hideable to match our use of TableSetupScrollFreeze()
        for (auto i = 0; i < _tracks.size(); ++i) {
            ImGui::TableSetupColumn(_tracks[i]->_name.c_str());
        }
        ImGui::TableHeadersRow();
        for (int line = 0; line < _maxLine; line++) {
            ImGui::TableNextRow();
            if (line == 0) {
                ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.7f, 0.65f));
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, cell_bg_color);
            }
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%02d", line);
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
    _lines[0]->assign("C4");
    _lines[1]->assign("OFF");
    _lines[2]->assign("E4");
    _lines[3]->assign("OFF");
    _lines[4]->assign("G4");
    _lines[5]->assign("OFF");
}

Track::~Track()
{
    if (_out != nullptr) {
        free(_out);
    }
}

void Track::process(AudioBuffer* in, unsigned long framesPerBuffer, int64_t steadyTime)
{
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
            in->_eventIn.noteOff(_lastKey, 0, 100, 0);
            continue;
        }
        in->_eventIn.noteOn(key, 0, 100, 0);
        _lastKey = key;

    }


    _audioBuffer.ensure(framesPerBuffer);
    AudioBuffer* buffer = in;
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
    ImGui::InputText("", p);
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

void AudioBuffer::copyOutToOutFrom(AudioBuffer* from)
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

PlayPosition PlayPosition::nextPlayPosition(double sampleRate, unsigned long framesPerBuffer, float bpm, int lpb) const
{
    double deltaSec = framesPerBuffer / sampleRate;
    double oneBeatSec = 60.0 / bpm;
    double oneLineSec = oneBeatSec / lpb;
    double quotient = std::floor(deltaSec / oneLineSec);
    double remainder = std::fmod(deltaSec, oneLineSec);
    int line = static_cast<int>(quotient) + _line;
    double oneDelaySec = oneLineSec / 0x100;
    auto delay = (std::floor(remainder / oneDelaySec)) + _delay;
    if (delay > 0xff) {
        ++line;
        delay -= 0x100;
    }

    return PlayPosition{ line,  static_cast<unsigned char>(delay) };
}

PlayPosition& PlayPosition::operator+=(const PlayPosition& rhs)
{
    _line += rhs._line;
    auto delay = _delay + rhs._delay;
    if (delay > 0xff) {
        _line += delay / 0x100;
        _delay = static_cast<unsigned char>(delay % 0x100);
    }
    else {
        _delay = static_cast<unsigned char>(delay);
    }
    return *this;
}
