#include "Composer.h"
#include <cstdlib>
#include <map>
#include <ranges>
#include <sstream>
#include "App.h"
#include "AudioEngine.h"
#include "AudioEngineWindow.h"
#include "Clip.h"
#include "Config.h"
#include "ErrorWindow.h"
#include "Fader.h"
#include "GuiUtil.h"
#include "Project.h"
#include "logger.h"
#include "Track.h"
#include "Lane.h"
#include "util.h"

Composer::Composer() :
    _commandManager(this),
    _project(std::make_unique<Project>(this)),
    _masterTrack(std::make_unique<Track>(std::string("MASTER"), this)),
    _composerWindow(std::make_unique<ComposerWindow>(this)),
    _rackWindow(std::make_unique<RackWindow>(this)),
    _sceneMatrix(std::make_unique<SceneMatrix>(this)),
    _timelineWindow(std::make_unique<TimelineWindow>(this)),
    _pianoRollWindow(std::make_unique<PianoRollWindow>(this)),
    _sideChainInputSelector(std::make_unique<SidechainInputSelector>(this)),
    _commandWindow(std::make_unique<CommandWindow>(this)) {
}

Composer::Composer(const nlohmann::json& json) :
    Nameable(json),
    _commandManager(this),
    _project(std::make_unique<Project>(this)),
    _composerWindow(std::make_unique<ComposerWindow>(this)),
    _rackWindow(std::make_unique<RackWindow>(this)),
    _timelineWindow(std::make_unique<TimelineWindow>(this)),
    _pianoRollWindow(std::make_unique<PianoRollWindow>(this)),
    _sideChainInputSelector(std::make_unique<SidechainInputSelector>(this)),
    _commandWindow(std::make_unique<CommandWindow>(this)) {

    _bpm = json["_bpm"];
    _playTime = json["_playTime"];
    _playStartTime = json["_playStartTime"];
    _loopStartTime = json["_loopStartTime"];
    _loopEndTime = json["_loopEndTime"];
    _looping = json["_looping"];
    _scrollLock = json["_scrollLock"];

    _sceneMatrix = std::make_unique<SceneMatrix>(json["_sceneMatrix"]);
    _sceneMatrix->_composer = this;

    _masterTrack = std::make_unique<Track>(json["_masterTrack"]);
    _masterTrack->setComposer(this);

    _masterTrack->resolveModuleReference();
}

void Composer::render() const {
    _composerWindow->render();
    _rackWindow->render();
    _sceneMatrix->render();
    _timelineWindow->render();
    _pianoRollWindow->render();
    _sideChainInputSelector->render();
    _commandWindow->render();
}

void Composer::process(float* /* in */, float* out, unsigned long framesPerBuffer, int64_t steadyTime) {
    // TODO オーディオ入力
    _processBuffer.clear();
    _processBuffer.ensure32();
    _processBuffer.ensure(framesPerBuffer, 1, 2);

    _masterTrack->prepare(framesPerBuffer);

    if (_playing) {
        computeNextPlayTime(framesPerBuffer);
    }
    _masterTrack->prepareEvent();

    for (auto& module : _orderedModules) {
        module->_track->_processBuffer.swapInOut();
        module->processConnections();
        module->process(&module->_track->_processBuffer, steadyTime);
    }

    //for (auto& track : getTracks()) {
    //    track->doDCP();
    //    _masterTrack->_processBuffer._in[0].add(track->_processBuffer._out[0]);
    //}

    //for (auto& module : _masterTrack->_modules) {
    //    module->_track->_processBuffer.swapInOut();
    //    module->processConnections();
    //    module->process(&module->_track->_processBuffer, steadyTime);
    //}
    //_masterTrack->_fader->process(&_masterTrack->_processBuffer, steadyTime);

    _masterTrack->_processBuffer._out[0].copyTo(out, framesPerBuffer, 2);

    if (_playing) {
        _playTime = _nextPlayTime;
    }
    if (_looping) {
        // TODO ループ時の端数処理
        if (_playTime >= _loopEndTime) {
            _playTime = _loopStartTime;
        }
    }
}

App* Composer::app() const {
    return _app;
}

AudioEngine* Composer::audioEngine() const {
    return app()->audioEngine();
}

void Composer::computeNextPlayTime(unsigned long framesPerBuffer) {
    double deltaSec = framesPerBuffer / gPreference.sampleRate;
    double oneBeatSec = 60.0 / _bpm;
    _nextPlayTime = deltaSec / oneBeatSec + _playTime;
}

int Composer::maxBar() {
    // TODO
    return 50;
}

void Composer::clear() {
    _masterTrack.reset(new Track(std::string("MASTER"), this));
    _orderedModules.clear();
    _commandManager.clear();
    _sceneMatrix->_scenes.clear();
    _pianoRollWindow->_show = false;
}

void Composer::computeProcessOrder() {
    std::vector<Module*> orderedModules;
    std::set<Module*> processedModules;
    std::map<Track*, Module*> waitingModule;

    while (true) {
        if (computeProcessOrder(_masterTrack, orderedModules, processedModules, waitingModule)) {
            break;
        }
    }

    std::lock_guard<std::recursive_mutex> lock(app()->_mtx);
    _orderedModules = orderedModules;
    computeLatency();
}

bool Composer::computeProcessOrder(std::unique_ptr<Track>& track,
                                   std::vector<Module*>& orderedModules,
                                   std::set<Module*>& processedModules,
                                   std::map<Track*, Module*> waitingModule) {
    bool processed = true;
    for (auto& x : track->getTracks()) {
        processed &= computeProcessOrder(x, orderedModules, processedModules, waitingModule);
    }

    std::vector<Module*> allModules = track->allModules();;

    bool skip = waitingModule.contains(track.get());
    for (auto& module : allModules) {
        if (skip) {
            if (module == waitingModule[track.get()]) {
                waitingModule.erase(track.get());
            } else {
                continue;
            }
        }
        if (module->isStarting()) {
            if (!processedModules.contains(module)) {
                for (auto& connection : module->_connections) {
                    if (connection->_to == module && !processedModules.contains(connection->_from) && connection->_from->isStarting()) {
                        waitingModule[track.get()] = module;
                        return false;
                    }

                }
                orderedModules.push_back(module);
                processedModules.insert(module);
            }
            for (auto& connection : module->_connections) {
                if (connection->_from == module && !processedModules.contains(connection->_to) && connection->_to->isStarting()) {
                    waitingModule[track.get()] = module;
                    return false;
                }
            }
        }
    }

    return processed;
}

void Composer::computeLatency() {
    std::lock_guard<std::recursive_mutex> lock(app()->_mtx);
    for (auto& module : _orderedModules) {
        uint32_t latency = module->_latency;
        for (auto& x : _orderedModules) {
            if (module == x) {
                break;
            }
            if (module->_track == x->_track) {
                latency = module->_latency + x->getComputedLatency();
            }
        }
        for (auto& connection : module->_connections) {
            if (connection->_to == module) {
                latency = std::max(latency,
                                   connection->_from->getComputedLatency());
            }
        }
        module->setComputedLatency(latency);
    }
}

nlohmann::json Composer::toJson() {
    nlohmann::json json = Nameable::toJson();

    json["_bpm"] = _bpm;
    json["_playTime"] = _playTime;
    json["_playStartTime"] = _playStartTime;
    json["_loopStartTime"] = _loopStartTime;
    json["_loopEndTime"] = _loopEndTime;
    json["_looping"] = _looping;
    json["_scrollLock"] = _scrollLock;

    json["_masterTrack"] = _masterTrack->toJson();

    json["_sceneMatrix"] = _sceneMatrix->toJson();

    return json;
}

void Composer::play() {
    if (_playing) {
        return;
    }
    _playing = true;
    _playStartTime = _playTime;
}

void Composer::stop() {
    if (!_playing) {
        return;
    }
    _playing = false;
    _playTime = _playStartTime;
    _nextPlayTime = _playStartTime;
    _sceneMatrix->stop();
}

std::vector<Track*> Composer::allTracks() const {
    std::vector<Track*> tracks;
    _masterTrack->allTracks(tracks);
    return tracks;
}
