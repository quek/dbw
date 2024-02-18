#include "Composer.h"
#include <cstdlib>
#include <map>
#include <ranges>
#include <sstream>
#include "AudioEngine.h"
#include "AudioEngineWindow.h"
#include "Clip.h"
#include "Command.h"
#include "ErrorWindow.h"
#include "GuiUtil.h"
#include "Project.h"
#include "logger.h"
#include "Track.h"
#include "Lane.h"
#include "util.h"

Composer::Composer(AudioEngine* audioEngine) :
    _audioEngine(audioEngine),
    _commandManager(this),
    _pluginManager(this),
    _project(std::make_unique<Project>(yyyyMmDd(), this)),
    _masterTrack(std::make_unique<Track>("MASTER", this)),
    _composerWindow(std::make_unique<ComposerWindow>(this)),
    _sceneMatrix(std::make_unique<SceneMatrix>(this)),
    _timelineWindow(std::make_unique<TimelineWindow>(this)),
    _pianoRollWindow(std::make_unique<PianoRollWindow>(this)),
    _sideChainInputSelector(std::make_unique<SidechainInputSelector>(this)) {
    addTrack();
    _pluginManager.load();
}

void Composer::render() const {
    _composerWindow->render();
    _sceneMatrix->render();
    _timelineWindow->render();
    _pianoRollWindow->render();
    _sideChainInputSelector->render();
}

void Composer::process(float* /* in */, float* out, unsigned long framesPerBuffer, int64_t steadyTime) {
    _masterTrack->prepare(framesPerBuffer);
    for (auto& track : _tracks) {
        track->prepare(framesPerBuffer);
    }

    if (_playing) {
        computeNextPlayTime(framesPerBuffer);
    }

    _processBuffer.clear();
    _processBuffer.ensure32();
    _processBuffer.ensure(framesPerBuffer, 1, 2);

    bool processed = false;
    while (!processed) {
        processed = true;
        for (auto& track : _tracks) {
            processed &= track->process(steadyTime);
        }
    }

    _masterTrack->_processBuffer.inZero();
    for (auto& track : _tracks) {
        _masterTrack->_processBuffer._in[0].add(track->_processBuffer._out[0]);
    }

    _masterTrack->process(steadyTime);
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
    double deltaSec = framesPerBuffer / _audioEngine->_sampleRate;
    double oneBeatSec = 60.0 / _bpm;
    _nextPlayTime = deltaSec / oneBeatSec + _playTime;
}

void Composer::scanPlugin() {
    _pluginManager.scan();
}

int Composer::maxBar() {
    // TODO
    return 50;
}

void Composer::clear() {
    _tracks.clear();
    _commandManager.clear();
    _sceneMatrix->_scenes.clear();
    _pianoRollWindow->_show = false;
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
        std::lock_guard<std::mutex> lock(composer->_audioEngine->mtx);
        composer->_tracks.push_back(std::move(_track));
    }
    void undo(Composer* composer) override {
        std::lock_guard<std::mutex> lock(composer->_audioEngine->mtx);
        _track = std::move(composer->_tracks.back());
        composer->_tracks.pop_back();
    }

    std::unique_ptr<Track> _track;
};

void Composer::addTrack() {
    std::stringstream name;
    name << "track " << this->_tracks.size() + 1;
    Track* track = new Track(name.str(), this);
    _commandManager.executeCommand(new AddTrackCommand(track));
}
