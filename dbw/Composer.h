#pragma once
#include <memory>
#include <set>
#include "ProcessBuffer.h"
#include "Command.h"
#include "ComposerWindow.h"
#include "Midi.h"
#include "PianoRollWindow.h"
#include "PluginManager.h"
#include "Project.h"
#include "SceneMatrix.h"
#include "SidechainInputSelector.h"
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
    void addTrack(Track* track);
    void scanPlugin();
    int maxBar();
    void clear();
    void computeProcessOrder();
    void computeLatency();

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
    bool _isScrollFolloPlayhead = true;
    CommandManager _commandManager;
    std::vector<std::unique_ptr<Track>> _tracks;
    std::unique_ptr<Track> _masterTrack;
    std::vector<Module*> _orderedModules;
    uint32_t _maxLatency;

    std::unique_ptr<ComposerWindow> _composerWindow;
    std::unique_ptr<SceneMatrix> _sceneMatrix;
    std::unique_ptr<TimelineWindow> _timelineWindow;
    std::unique_ptr<PianoRollWindow> _pianoRollWindow;
    std::unique_ptr<SidechainInputSelector> _sideChainInputSelector;
private:
};
