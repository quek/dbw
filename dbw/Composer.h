#pragma once
#include <memory>
#include "ProcessBuffer.h"
#include "Command.h"
#include "MasterTrack.h"
#include "Midi.h"
#include "PlayPosition.h"
#include "PluginManager.h"
#include "Project.h"
#include "Track.h"

class AudioEngine;

class Composer
{
public:
    Composer(AudioEngine* audioEngine);
    void process(float* in, float* out, unsigned long framesPerBuffer, int64_t steadyTime);

    void play();
    void stop();
    void addTrack();
    void changeMaxLine();
    void scanPlugin();
    void setStatusMessage(std::string message);

    std::unique_ptr<Project> _project;
    AudioEngine* _audioEngine;
    ProcessBuffer _processBuffer;
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
    std::vector<std::unique_ptr<Track>> _tracks;
    std::unique_ptr<MasterTrack> _masterTrack;

private:
};

