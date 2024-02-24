#include "Composer.h"
#include <cstdlib>
#include <map>
#include <ranges>
#include <sstream>
#include "App.h"
#include "AudioEngine.h"
#include "AudioEngineWindow.h"
#include "Clip.h"
#include "Command.h"
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
    _masterTrack(std::make_unique<Track>(std::string("MASTER"))),
    _composerWindow(std::make_unique<ComposerWindow>(this)),
    _rackWindow(std::make_unique<RackWindow>(this)),
    _sceneMatrix(std::make_unique<SceneMatrix>(this)),
    _timelineWindow(std::make_unique<TimelineWindow>(this)),
    _pianoRollWindow(std::make_unique<PianoRollWindow>(this)),
    _sideChainInputSelector(std::make_unique<SidechainInputSelector>(this)),
    _commandWindow(std::make_unique<CommandWindow>(this)) {
    _masterTrack->_composer = this;
    addTrack();
}

Composer::Composer(const nlohmann::json& json) :
    TracksHolder(json),
    _commandManager(this),
    _project(std::make_unique<Project>(this)),
    _composerWindow(std::make_unique<ComposerWindow>(this)),
    _rackWindow(std::make_unique<RackWindow>(this)),
    _timelineWindow(std::make_unique<TimelineWindow>(this)),
    _pianoRollWindow(std::make_unique<PianoRollWindow>(this)),
    _sideChainInputSelector(std::make_unique<SidechainInputSelector>(this)),
    _commandWindow(std::make_unique<CommandWindow>(this)) {

    _bpm = json["_bpm"];

    _sceneMatrix = std::make_unique<SceneMatrix>(json["_sceneMatrix"]);
    _sceneMatrix->_composer = this;

    _masterTrack = std::make_unique<Track>(json["_masterTrack"]);
    _masterTrack->_composer = this;

    for (const auto& x : json["_tracks"]) {
        Track* track = new Track(x);
        track->_composer = this;
        addTrack(track);
    }

    for (const auto& module : _masterTrack->_modules) {
        for (const auto& connection : module->_connections) {
            connection->resolveModuleReference();
        }
    }
    for (const auto& track : getTracks()) {
        for (const auto& module : track->_modules) {
            for (const auto& connection : module->_connections) {
                connection->resolveModuleReference();
            }
        }
    }
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
    for (auto& track : getTracks()) {
        track->prepare(framesPerBuffer);
    }
    _masterTrack->prepare(framesPerBuffer);

    if (_playing) {
        computeNextPlayTime(framesPerBuffer);
    }

    _processBuffer.clear();
    _processBuffer.ensure32();
    _processBuffer.ensure(framesPerBuffer, 1, 2);

    for (auto& track : getTracks()) {
        track->prepareEvent();
    }

    for (auto& module : _orderedModules) {
        module->_track->_processBuffer.swapInOut();
        module->processConnections();
        module->process(&module->_track->_processBuffer, steadyTime);
    }

    for (auto& track : getTracks()) {
        track->doDCP();
        _masterTrack->_processBuffer._in[0].add(track->_processBuffer._out[0]);
    }

    for (auto& module : _masterTrack->_modules) {
        module->_track->_processBuffer.swapInOut();
        module->processConnections();
        module->process(&module->_track->_processBuffer, steadyTime);
    }
    _masterTrack->_fader->process(&_masterTrack->_processBuffer, steadyTime);
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
    getTracks().clear();
    _orderedModules.clear();
    _commandManager.clear();
    _sceneMatrix->_scenes.clear();
    _pianoRollWindow->_show = false;
}

void Composer::computeProcessOrder() {
    std::vector<Module*> orderedModules;
    std::set<Module*> processedModules;
    Module* _waitingModule = nullptr;

    bool processed = false;
    while (!processed) {
        processed = true;
        for (auto& track : getTracks()) {
            bool skip = _waitingModule && _waitingModule->_track == track.get();
            for (auto& module : track->_modules) {
                if (skip) {
                    if (module.get() == _waitingModule) {
                        skip = false;
                    } else {
                        continue;
                    }
                }
                if (module->isStarting()) {
                    if (!processedModules.contains(module.get())) {
                        bool isWaitingFrom = false;
                        for (auto& connection : module->_connections) {
                            if (connection->_to == module.get() && !processedModules.contains(connection->_from) && connection->_from->isStarting()) {
                                isWaitingFrom = true;
                                break;
                            }

                        }
                        if (isWaitingFrom) {
                            _waitingModule = module.get();
                            processed = false;
                            break;
                        }
                        orderedModules.push_back(module.get());
                        processedModules.insert(module.get());
                    }
                    bool isWaitingTo = false;
                    for (auto& connection : module->_connections) {
                        if (connection->_from == module.get() && !processedModules.contains(connection->_to) && connection->_to->isStarting()) {
                            isWaitingTo = true;
                            break;
                        }
                    }
                    if (isWaitingTo) {
                        _waitingModule = module.get();
                        processed = false;
                        break;
                    }
                }
            }
            if (processed) {
                Module* fader = (Module*)track->_fader.get();
                orderedModules.push_back(fader);
                processedModules.insert(fader);
            }
        }
    }
    std::lock_guard<std::recursive_mutex> lock(app()->_mtx);
    _orderedModules = orderedModules;
    computeLatency();
}

void Composer::computeLatency() {
    for (auto& module : _orderedModules) {
        uint32_t latency = module->_latency;
        for (auto& x : _orderedModules) {
            if (module == x) {
                break;
            }
            if (module->_track == x->_track) {
                latency += x->getComputedLatency();
            }
        }
        for (auto& connection : module->_connections) {
            if (connection->_to == module) {
                latency += connection->_from->getComputedLatency();
            }
        }
        module->setComputedLatency(latency);
    }

    uint32_t maxLatency = 0;
    for (const auto& track : getTracks()) {
        uint32_t latency = track->computeLatency();
        if (maxLatency < latency) {
            maxLatency = latency;
        }
    }
    {
        std::lock_guard<std::recursive_mutex> lock(app()->_mtx);
        for (const auto& track : getTracks()) {
            track->_processBuffer.setLatency(maxLatency - track->_latency);
        }
    }
    // マスタはレイテンシー出すだけでいいかな
    _masterTrack->computeLatency();
}

void Composer::deleteClips(std::set<Clip*> clips) {
    // TODO undo
    for (auto clip : clips) {
        for (auto& track : getTracks()) {
            for (auto& lane : track->_lanes) {
                auto it = std::ranges::find_if(lane->_clips, [clip](const auto& x) { return x.get() == clip; });
                if (it != lane->_clips.end()) {
                    lane->_clips.erase(it);
                    break;
                }
            }
        }
    }
}

nlohmann::json Composer::toJson() {
    nlohmann::json json = Nameable::toJson();

    json["_bpm"] = _bpm;

    json["_masterTrack"] = _masterTrack->toJson();

    nlohmann::json tracks = nlohmann::json::array();
    for (const auto& track : getTracks()) {
        tracks.emplace_back(track->toJson());
    }
    json["_tracks"] = tracks;

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

class AddTrackCommand : public Command {
public:
    AddTrackCommand(Track* track) : _track(track) {}
    void execute(Composer* composer) override {
        std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
        _track->_composer = composer;
        composer->addTrack(std::move(_track));
        composer->computeProcessOrder();
    }
    void undo(Composer* composer) override {
        std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
        _track = std::move(composer->getTracks().back());
        composer->getTracks().pop_back();
        composer->computeProcessOrder();
    }

    std::unique_ptr<Track> _track;
};

std::vector<Track*> Composer::allTracks() {
    std::vector<Track*> tracks{ _masterTrack.get() };
    for (auto& track : getTracks()) {
        tracks.push_back(track.get());
    }
    return tracks;
}
