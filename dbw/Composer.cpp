#include "Composer.h"
#include <cstdlib>
#include <map>
#include <ranges>
#include <sstream>
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

Composer::Composer(AudioEngine* audioEngine) :
    _audioEngine(audioEngine),
    _commandManager(this),
    _project(std::make_unique<Project>(yyyyMmDd(), this)),
    _masterTrack(std::make_unique<Track>("MASTER", this)),
    _composerWindow(std::make_unique<ComposerWindow>(this)),
    _sceneMatrix(std::make_unique<SceneMatrix>(this)),
    _timelineWindow(std::make_unique<TimelineWindow>(this)),
    _pianoRollWindow(std::make_unique<PianoRollWindow>(this)),
    _sideChainInputSelector(std::make_unique<SidechainInputSelector>(this)) {
    addTrack();
}

void Composer::render() const {
    _composerWindow->render();
    _sceneMatrix->render();
    _timelineWindow->render();
    _pianoRollWindow->render();
    _sideChainInputSelector->render();
}

void Composer::process(float* /* in */, float* out, unsigned long framesPerBuffer, int64_t steadyTime) {
    for (auto& track : _tracks) {
        track->prepare(framesPerBuffer);
    }
    _masterTrack->prepare(framesPerBuffer);

    if (_playing) {
        computeNextPlayTime(framesPerBuffer);
    }

    _processBuffer.clear();
    _processBuffer.ensure32();
    _processBuffer.ensure(framesPerBuffer, 1, 2);

    for (auto& track : _tracks) {
        track->prepareEvent();
    }

    for (auto& module : _orderedModules) {
        module->_track->_processBuffer.swapInOut();
        module->processConnections();
        module->process(&module->_track->_processBuffer, steadyTime);
    }

    for (auto& track : _tracks) {
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
    _tracks.clear();
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
        for (auto& track : _tracks) {
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
    std::lock_guard<std::recursive_mutex> lock(_audioEngine->_mtx);
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
    for (const auto& track : _tracks) {
        uint32_t latency = track->computeLatency();
        if (maxLatency < latency) {
            maxLatency = latency;
        }
    }
    {
        std::lock_guard<std::recursive_mutex> lock(_audioEngine->_mtx);
        _maxLatency = maxLatency;
        for (const auto& track : _tracks) {
            track->_processBuffer.setLatency(maxLatency - track->_latency);
        }
    }
    // マスタはレイテンシー出すだけでいいかな
    _masterTrack->computeLatency();
}

void Composer::deleteClips(std::set<Clip*> clips) {
    // TODO undo
    for (auto clip : clips) {
        for (auto& track : _tracks) {
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
        std::lock_guard<std::recursive_mutex> lock(composer->_audioEngine->_mtx);
        _track->_composer = composer;
        composer->_tracks.push_back(std::move(_track));
        composer->computeProcessOrder();
    }
    void undo(Composer* composer) override {
        std::lock_guard<std::recursive_mutex> lock(composer->_audioEngine->_mtx);
        _track = std::move(composer->_tracks.back());
        composer->_tracks.pop_back();
        composer->computeProcessOrder();
    }

    std::unique_ptr<Track> _track;
};

void Composer::addTrack() {
    auto name = std::to_string(this->_tracks.size() + 1);
    Track* track = new Track(name, this);
    addTrack(track);
}

void Composer::addTrack(Track* track) {
    _commandManager.executeCommand(new AddTrackCommand(track));
}
