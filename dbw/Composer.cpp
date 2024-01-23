#include "Composer.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "PluginHost.h"
#include "logging.h"
#include <sstream>
#include <cstdlib>



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
        _playPosition += _nextPlayPosition;
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
    // TODO DELETE
    clap_event_note event;
    if (_composer->_playing) {
        in->_eventIn.noteOn(60, 0, 100, 0);
        in->_eventIn.noteOff(60, 0, 100, 1024);
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

PlayPosition PlayPosition::nextPlayPosition(double sampleRate, unsigned long framesPerBuffer, float bpm, int lpb)
{
    double deltaSec = framesPerBuffer / sampleRate;
    double oneBeatSec = 60.0 / bpm;
    double oneLineSec = oneBeatSec / lpb;
    double quotient = std::floor(deltaSec / oneLineSec);
    double remainder = std::fmod(deltaSec, oneLineSec);
    int line = static_cast<int>(quotient);
    unsigned char delay = static_cast<unsigned char>(std::floor(remainder / (oneLineSec / 0xff)));

    return PlayPosition{ line,  delay };
}

PlayPosition& PlayPosition::operator+=(const PlayPosition& rhs)
{
    _line += rhs._line;
    auto delay = _delay + rhs._delay;
    if (delay > 0xff) {
        ++_line;
        _delay = delay - 0x100;
    }
    else {
        _delay = delay;
    }
    return *this;
}
