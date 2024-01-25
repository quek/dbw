#pragma once
#include "AudioEngine.h"
#include "PluginHost.h"
#include "PlayPosition.h"
#include "Midi.h"
#include "Command.h"
#include "PluginManager.h"
#include <memory>

class Composer;
class Line;

class AudioBuffer {
public:
    AudioBuffer();
    AudioBuffer(unsigned long framesPerBuffer);
    void ensure(unsigned long framesPerBuffer);
    std::unique_ptr<float[]> _in;
    std::unique_ptr<float[]> _out;
    unsigned long _framesPerBuffer;
    void copyInToInFrom(const AudioBuffer* from);
    void copyOutToOutFrom(const AudioBuffer* from);
    PluginEventList _eventIn;
    PluginEventList _eventOut;

    void clear();
};

class Module {
public:
    virtual ~Module() = default;
    void openGui() {};
    void closeGui() {};
    virtual void render();
    virtual void process(AudioBuffer* in, unsigned long framesPerBuffer, int64_t steadyTime);
    AudioBuffer _audioBuffer;
};

class PluginModule : public Module {
public:
    PluginModule(PluginHost* pluginHost);
    ~PluginModule();
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
    Track(std::string name, Composer* composer);
    ~Track();
    void process(const AudioBuffer* in, unsigned long framesPerBuffer, int64_t steadyTime);
    void render();
    void renderLine(int line);
    void changeMaxLine(int value);
    AudioBuffer _audioBuffer;
    float* _out = nullptr;

    std::string _name;
    std::vector<std::unique_ptr<Clip>> _clips;
    std::vector<std::unique_ptr<Line>> _lines;
    std::vector<std::unique_ptr<Module>> _modules;
    int16_t _lastKey = NOTE_NONE;

    std::string _pluginPath = { "C:\\Program Files\\Common Files\\CLAP\\Surge Synth Team\\Surge XT.clap" };
    Composer* _composer;
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
    void changeMaxLine();
    void scanPlugin();

    AudioEngine* _audioEngine;
    AudioBuffer _audioBuffer;
    float _bpm = 128.0;
    int _lpb = 4;
    int _samplePerDelay;
    bool _playing = false;
    bool _looping = false;
    bool _scrollLock = false;
    int _maxLine = 0x40;
    PlayPosition _playStartPosition{};
    PlayPosition _playPosition{};
    PlayPosition _nextPlayPosition{};
    PlayPosition _loopStartPosition{};
    PlayPosition _loopEndPosition{ ._line = 0x41, ._delay = 0 };
    CommandManager _commandManager;
    PluginManager _pluginManager;

private:

    // delete
    PluginHost* _pluginHost = nullptr;;
    std::string _pluginPath = { "C:\\Program Files\\Common Files\\CLAP\\Surge Synth Team\\Surge XT.clap" };


    std::vector<std::unique_ptr<Track>> _tracks;
};

