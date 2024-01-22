#pragma once
#include "AudioEngine.h"
#include "PluginHost.h"
#include <memory>

extern float TEXT_BASE_WIDTH;
extern float TEXT_BASE_HEIGHT;

class Composer;

class AudioBuffer {
public:
    AudioBuffer::AudioBuffer();
    AudioBuffer::AudioBuffer(unsigned long framesPerBuffer);
    void ensure(unsigned long framesPerBuffer);
    std::unique_ptr<float[]> _in;
    std::unique_ptr<float[]> _out;
    unsigned long _framesPerBuffer;
    void copyOutToOutFrom(AudioBuffer* from);
};

class Module {
public:
    void openGui() {};
    void closeGui() {};
    virtual void render();
    virtual void process(AudioBuffer* in, unsigned long framesPerBuffer, int64_t steadyTime);
    AudioBuffer _audioBuffer;
};

class PluginModule : public Module {
public:
    PluginModule::PluginModule(PluginHost* pluginHost);
    PluginModule::~PluginModule();
    void openGui() const { _pluginHost->openGui(); };
    void closeUgi() const { _pluginHost->closeGui(); };
    void process(AudioBuffer* in, unsigned long framesPerBuffer, int64_t steadyTime) override;
    void render() override;

    std::unique_ptr<PluginHost> _pluginHost;
};

class MidiEvent {
};

class Clip {
    std::vector<std::unique_ptr<MidiEvent>> _midiEvents;
};

class Track {
public:
    Track::Track(std::string name, Composer* composer);
    Track::~Track();
    void process(AudioBuffer* in, unsigned long framesPerBuffer, int64_t steadyTime);
    void render();
    AudioBuffer _audioBuffer;
    float* _out = nullptr;

    std::string _name;
    std::vector<std::unique_ptr<Clip>> _clips;
    std::vector<std::unique_ptr<Module>> _modules;

    std::string _pluginPath = { "C:\\Program Files\\Common Files\\CLAP\\Surge Synth Team\\Surge XT.clap" };
    Composer* _composer;
};


class Position {
public:
    int bar = 0;
    int beat = 0;
    int tick = 0;
    static const int TICK_PER_BEAT = 960;
};

class Composer
{
public:
    Composer(AudioEngine* audioEngine);
    void process(float* in, float* out, unsigned long framesPerBuffer, int64_t steadyTime);
    void render();

    void play();
    void stop();
    void addTrack();

    AudioEngine* _audioEngine;
    AudioBuffer _audioBuffer;
private:

    // delete
    PluginHost* _pluginHost = nullptr;;
    std::string _pluginPath = { "C:\\Program Files\\Common Files\\CLAP\\Surge Synth Team\\Surge XT.clap" };

    float bpm{ 128.0 };
    Position _playPosition{};

    std::vector<std::unique_ptr<Track>> _tracks;
};

