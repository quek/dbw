#include "Composer.h"
#include <cstdlib>
#include <map>
#include <ranges>
#include <sstream>
#include "AudioEngine.h"
#include "Column.h"
#include "Command.h"
#include "ErrorWindow.h"
#include "GuiUtil.h"
#include "Line.h"
#include "Project.h"
#include "logger.h"
#include "Track.h"
#include "util.h"

Composer::Composer(AudioEngine* audioEngine) :
    _audioEngine(audioEngine),
    _commandManager(this),
    _pluginManager(this),
    _project(std::make_unique<Project>("noname", this)),
    _masterTrack(std::make_unique<MasterTrack>(this)) {
    addTrack();
}

void Composer::process(float* /* in */, float* out, unsigned long framesPerBuffer, int64_t steadyTime) {
    _masterTrack->_processBuffer.clear();
    _masterTrack->_processBuffer.ensure(framesPerBuffer, 2);

    if (_playing) {
        _nextPlayPosition = _playPosition.nextPlayPosition(_audioEngine->_sampleRate, framesPerBuffer, _bpm, _lpb, &_samplePerDelay);
    }

    _processBuffer.clear();
    _processBuffer.ensure(framesPerBuffer, 2);

    _masterTrack->_processBuffer.ensure(framesPerBuffer, 2);
    _masterTrack->_processBuffer._in.zero();
    for (auto iter = _tracks.begin(); iter != _tracks.end(); ++iter) {
        auto& track = *iter;
        track->_processBuffer.clear();
        track->_processBuffer.ensure(framesPerBuffer, 2);
        track->process(steadyTime);
        _masterTrack->_processBuffer._in.add(track->_processBuffer._out);
    }

    _masterTrack->process(steadyTime);
    _masterTrack->_processBuffer._out.copyTo(out, framesPerBuffer, 2);

    if (_playing) {
        _playPosition = _nextPlayPosition;
    }
    if (_looping) {
        if (_playPosition >= _loopEndPosition) {
            _playPosition = _loopStartPosition;
        }
    }
    if (_playPosition._line > _maxLine) {
        _playPosition._line = 0;
        _playPosition._delay = 0;
    }
}

void Composer::scanPlugin() {
    _pluginManager.scan();
}

void Composer::changeMaxLine() {
    for (auto track = _tracks.begin(); track != _tracks.end(); ++track) {
        (*track)->changeMaxLine(_maxLine);
    }
}

void Composer::play() {
    _playing = true;
}

void Composer::stop() {
    _playing = false;
    _playPosition = PlayPosition{ ._line = 0, ._delay = 0 };
    _nextPlayPosition = PlayPosition{ ._line = 0, ._delay = 0 };
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
