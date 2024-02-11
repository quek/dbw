#pragma once
#include <memory>
#include <set>
#include "ProcessBuffer.h"
#include "Command.h"
#include "ComposerWindow.h"
#include "MasterTrack.h"
#include "Midi.h"
#include "PianoRoll.h"
#include "PluginManager.h"
#include "Project.h"
#include "SceneMatrix.h"
#include "TimelineWindow.h"
#include "Track.h"

class AudioEngine;

class Composer {
public:
    Composer(AudioEngine* audioEngine);
    void render() const;
    void process(float* in, float* out, unsigned long framesPerBuffer, int64_t steadyTime);
    void computeNextPlayTime(unsigned long framesPerBuffer);

    void play();
    void stop();
    void addTrack();
    void scanPlugin();
    int maxBar();

    void deleteClips(std::set<Clip*> clips);

    std::unique_ptr<Project> _project;
    AudioEngine* _audioEngine;
    ProcessBuffer _processBuffer;
    float _bpm = 128.0;
    int _samplePerDelay;
    bool _playing = false;
    bool _looping = false;
    bool _scrollLock = false;
    double _playStartTime = 0.0;
    double _playTime = 0.0;
    double _nextPlayTime = 0.0;
    double _loopStartTime = 0.0;
    double _loopEndTime = 16.0;
    CommandManager _commandManager;
    PluginManager _pluginManager;
    std::vector<std::unique_ptr<Track>> _tracks;
    std::unique_ptr<MasterTrack> _masterTrack;

    std::unique_ptr<ComposerWindow> _composerWindow;
    std::unique_ptr<SceneMatrix> _sceneMatrix;
    std::unique_ptr<TimelineWindow> _timelineWindow;
    std::unique_ptr<PianoRoll> _pianoRoll;
private:
};
