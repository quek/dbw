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
    _fader(new Fader("Fader", this)), Nameable(name), _composer(composer) {
    addLane(new Lane());
    _fader->start();
}

Track::~Track() {
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
}

void Track::prepareEvent() {
    double oneBeatSec = 60.0 / _composer->_bpm;
    double sampleRate = gPreference.sampleRate;
    for (auto& lane : _lanes) {
        for (auto& clip : lane->_clips) {
            double clipTime = clip->_time;
            double clipDuration = clip->_duration;
            double begin = _composer->_playTime;
            double end = _composer->_nextPlayTime;
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

    _composer->_sceneMatrix->process(this);
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
    _composer->computeProcessOrder();
}

void Track::addModule(Module* module) {
    module->_track = this;
    _modules.emplace_back(module);
    module->start();
    if (_composer) {
        _composer->computeProcessOrder();
    }
}

void Track::addLane(Lane* lane) {
    lane->_track = this;
    _lanes.emplace_back(lane);
}

void Track::addChild(Track* child) {
    child->_parent = this;
    _children.emplace_back(child);
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

