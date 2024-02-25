#include "Track.h"
#include <algorithm>
#include <mutex>
#include <ranges>
#include "imgui.h"
#include "AudioEngine.h"
#include "Clip.h"
#include "Command.h"
#include "Composer.h"
#include "Config.h"
#include "Fader.h"
#include "Midi.h"
#include "Module.h"
#include "ClapModule.h"
#include "ClapHost.h"
#include "Lane.h"
#include "command/AddModule.h"

Track::Track(const nlohmann::json& json) : Nameable(json) {
    _width = json["_width"];
    for (const auto& x : json["_lanes"]) {
        addLane(new Lane(x));
    }

    _fader.reset(new Fader(json["_fader"]));
    _fader->_track = this;
    if (json.contains("_modules")) {
        for (const auto& x : json["_modules"]) {
            addModule(gPluginManager.create(x));
        }
    }
}

Track::Track(std::string name, Composer* composer) :
    Nameable(name), _fader(new Fader("Fader", this)), _composer(composer) {
    addLane(new Lane());
    _fader->start();
}

Track::~Track() {
}

Composer* Track::getComposer() {
    if (_composer) {
        return _composer;
    }
    return _parent->getComposer();
}

void Track::setComposer(Composer* composer) {
    _composer = composer;
}

void Track::prepare(unsigned long framesPerBuffer) {
    _processBuffer.clear();
    int nbuses = 1;
    for (auto& module : _modules) {
        if (nbuses < module->_ninputs) {
            nbuses = module->_ninputs;
        }
        if (nbuses < module->_noutputs) {
            nbuses = module->_noutputs;
        }
    }
    _processBuffer.ensure(framesPerBuffer, nbuses, 2);

    for (const auto& track : _tracks) {
        track->prepare(framesPerBuffer);
    }
}

void Track::prepareEvent() {
    double oneBeatSec = 60.0 / getComposer()->_bpm;
    double sampleRate = gPreference.sampleRate;
    for (auto& lane : _lanes) {
        for (auto& clip : lane->_clips) {
            double clipTime = clip->_time;
            double clipDuration = clip->_duration;
            double begin = getComposer()->_playTime;
            double end = getComposer()->_nextPlayTime;
            // TODO Loop
            if (begin < clipTime + clipDuration && clipTime < end) {
                for (auto& note : clip->_sequence->_notes) {
                    double noteTime = clipTime + note->_time;
                    if (begin <= noteTime && noteTime < end) {
                        int16_t channel = 0;
                        uint32_t sampleOffsetDouble = (noteTime - begin) * oneBeatSec * sampleRate;
                        uint32_t sampleOffset = std::round(sampleOffsetDouble);
                        _processBuffer._eventOut.noteOn(note->_key, channel, note->_velocity, sampleOffset);
                    }
                    double noteDuration = noteTime + note->_duration;
                    if (begin <= noteDuration && noteDuration < end) {
                        int16_t channel = 0;
                        uint32_t sampleOffsetDouble = (noteDuration - begin) * oneBeatSec * sampleRate;
                        uint32_t sampleOffset = std::round(sampleOffsetDouble);
                        _processBuffer._eventOut.noteOff(note->_key, channel, 1.0f, sampleOffset);
                    }
                }
            }
        }
    }

    getComposer()->_sceneMatrix->process(this);

    for (const auto& track : _tracks) {
        track->prepareEvent();
    }
}


void Track::render() {
    ImGui::PushID(this);
    for (auto& module : _modules) {
        module->render();
    }
    if (ImGui::Button("Add Module")) {
        _openModuleSelector = true;
    }
    if (_openModuleSelector) {
        gPluginManager.openModuleSelector(this);
    }
    _fader->render();
    ImGui::Text(std::to_string(_latency).c_str());
    ImGui::PopID();
}

void Track::addModule(std::string path, uint32_t index) {
    ClapHost* pluginHost = new ClapHost(this);
    pluginHost->load(path.c_str(), index);
    Module* module = new ClapModule(pluginHost->_name, this, pluginHost);
    // TODO
    _modules.emplace_back(module);
    module->start();
    module->openGui();
    getComposer()->computeProcessOrder();
}

void Track::addModule(Module* module) {
    module->_track = this;
    _modules.emplace_back(module);
    module->start();
    if (getComposer()) {
        getComposer()->computeProcessOrder();
    }
}

void Track::addLane(Lane* lane) {
    lane->_track = this;
    _lanes.emplace_back(lane);
}

bool Track::isAvailableSidechainSrc(Track* dst) {
    if (this == dst) {
        // 当該モジュールより前のは使える
        return true;
    }
    // TODO
    return true;
}

uint32_t Track::computeLatency() {
    _latency = 0;
    for (auto& module : _modules) {
        _latency += module->getComputedLatency();
    }
    return _latency;
}

void Track::doDCP() {
    _processBuffer.doDCP();
}

nlohmann::json Track::toJson() {
    nlohmann::json json = Nameable::toJson();
    json["type"] = TYPE;
    json["_width"] = _width;
    json["_fader"].update(_fader->toJson());

    nlohmann::json modules = nlohmann::json::array();
    for (auto& module : _modules) {
        modules.emplace_back(module->toJson());
    }
    json["_modules"] = modules;

    nlohmann::json lanes = nlohmann::json::array();
    for (auto& lane : _lanes) {
        lanes.emplace_back(lane->toJson());
    }
    json["_lanes"] = lanes;

    return json;
}

void Track::addTrack() {
    std::string name = std::to_string(_tracks.size() + 1);
    Track* track = new Track(name);
    addTrack(std::unique_ptr<Track>(track));
}

void Track::addTrack(Track* track) {
    addTrack(std::unique_ptr<Track>(track));
}

void Track::addTrack(std::unique_ptr<Track> track) {
    track->_parent = this;
    _tracks.emplace_back(std::move(track));
}

Track* Track::getParent() {
    return _parent;
}

void Track::setParent(Track* parent) {
    _parent = parent;
}

void Track::resolveModuleReference() {
    for (const auto& module : _modules) {
        for (const auto& connection : module->_connections) {
            connection->resolveModuleReference();
        }
    }
    for (const auto& track : _tracks) {
        track->resolveModuleReference();
    }
}

void Track::play(Scene* scene) {
    for (const auto& lane : _lanes) {
        auto& clipSlot = lane->getClipSlot(scene);
        clipSlot->play();
    }
    for (const auto& track : _tracks) {
        track->play(scene);
    }
}

void Track::stop(Scene* scene) {
    for (const auto& lane : _lanes) {
        auto& clipSlot = lane->getClipSlot(scene);
        clipSlot->stop();
    }
    for (const auto& track : _tracks) {
        track->stop(scene);
    }
}

bool Track::isAllLanesPlaying(Scene* scene) {
    for (const auto& lane : _lanes) {
        auto& clipSlot = lane->getClipSlot(scene);
        if (!clipSlot->_playing) {
            return false;
        }
    }
    for (const auto& track : _tracks) {
        if (!track->isAllLanesPlaying(scene)) {
            return false;
        }
    }
    return true;
}

bool Track::isAllLanesStoped(Scene* scene) {
    for (const auto& lane : _lanes) {
        auto& clipSlot = lane->getClipSlot(scene);
        if (clipSlot->_playing) {
            return false;
        }
    }
    for (const auto& track : _tracks) {
        if (track->isAllLanesPlaying(scene)) {
            return false;
        }
    }
    return true;
}

void Track::allTracks(std::vector<Track*>& tracks) {
    tracks.push_back(this);
    for (const auto& x : _tracks) {
        x->allTracks(tracks);
    }
}

std::vector<std::unique_ptr<Track>>::iterator Track::findTrack(Track* track) {
    return std::ranges::find_if(_tracks, [track](const auto& x) { return x.get() == track; });
}

std::vector<std::unique_ptr<Track>>& Track::getTracks() {
    return _tracks;
}

