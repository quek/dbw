#pragma once
#include "AudioEngine.h"
#include "PluginHost.h"
#include <memory>


class Module {
};

class PluginModule : public Module {
    std::unique_ptr<PluginHost> _pluginHost;
};

class MidiEvent {
};

class Clip {
    std::vector<std::unique_ptr<MidiEvent>> _midiEvents;
};

class Track {
    std::vector<std::unique_ptr<Clip>> _clips;
    std::vector<std::unique_ptr<Module>> _plugins;
};


class PlayPosition {
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
    void render();

    void play();
    void stop();

private:
    AudioEngine* _audioEngine;

    // delete
    PluginHost* _pluginHost = nullptr;;
    std::string _pluginPath = { "C:\\Program Files\\Common Files\\CLAP\\Surge Synth Team\\Surge XT.clap" };

    float bpm{ 128.0 };
    PlayPosition _playPosition{};

    std::vector<std::unique_ptr<Track>> _tracks;
};

