#pragma once
#include <memory>
#include <set>
#include "ProcessBuffer.h"
#include "Command.h"
#include "CommandWindow.h"
#include "ComposerWindow.h"
#include "Midi.h"
#include "PianoRollWindow.h"
#include "PluginManager.h"
#include "Project.h"
#include "RackWindow.h"
#include "SceneMatrix.h"
#include "SidechainInputSelector.h"
#include "TimelineWindow.h"
#include "Track.h"

class App;
class AudioEngine;

class Composer : public Nameable {
public:
    Composer();
    Composer(const nlohmann::json& json);
    void render() const;
    void process(float* in, float* out, unsigned long framesPerBuffer, int64_t steadyTime);
    App* app() const;
    AudioEngine* audioEngine() const;
    void computeNextPlayTime(unsigned long framesPerBuffer);

    void play();
    void stop();
    std::vector<Track*> allTracks() const;
    int maxBar();
    void clear();
    void computeProcessOrder();
    bool computeProcessOrder(std::unique_ptr<Track>& track,
                             std::vector<Module*>& orderedModules,
                             std::set<Module*>& processedModules,
                             std::map<Track*, Module*> waitingModule);
    void computeLatency();
    virtual nlohmann::json toJson() override;

    App* _app = nullptr;
    std::unique_ptr<Project> _project;
    ProcessBuffer _processBuffer;
    float _bpm = 128.0;
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
    std::unique_ptr<Track> _masterTrack;
    std::vector<Module*> _orderedModules;

    std::unique_ptr<ComposerWindow> _composerWindow;
    std::unique_ptr<RackWindow> _rackWindow;
    std::unique_ptr<SceneMatrix> _sceneMatrix;
    std::unique_ptr<TimelineWindow> _timelineWindow;
    std::unique_ptr<PianoRollWindow> _pianoRollWindow;
    std::unique_ptr<SidechainInputSelector> _sideChainInputSelector;
    std::unique_ptr<CommandWindow> _commandWindow;
private:
};
