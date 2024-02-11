#include "Track.h"
#include <mutex>
#include "imgui.h"
#include "AudioEngine.h"
#include "Command.h"
#include "Composer.h"
#include "Midi.h"
#include "Module.h"
#include "PluginModule.h"
#include "PluginHost.h"
#include "TrackLane.h"

Track::Track(std::string name, Composer* composer) : _name(name), _composer(composer) {
    _trackLanes.emplace_back(new TrackLane());
    _lastKeys.push_back(0);
}

Track::~Track() {
}

void Track::process(int64_t steadyTime) {
    double oneBeatSec = 60.0 / _composer->_bpm;
    double sampleRate = _composer->_audioEngine->_sampleRate;
    for (auto& lane : _trackLanes) {
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
                        _processBuffer._eventIn.noteOn(note->_key, channel, note->_velocity, sampleOffset);
                    }
                    double noteDuration = noteTime + note->_duration;
                    if (begin <= noteDuration && noteDuration < end) {
                        int16_t channel = 0;
                        uint32_t sampleOffsetDouble = (noteDuration - begin) * oneBeatSec * sampleRate;
                        uint32_t sampleOffset = std::round(sampleOffsetDouble);
                        _processBuffer._eventIn.noteOff(note->_key, channel, 1.0f, sampleOffset);
                    }
                }
            }
        }
    }

    _composer->_sceneMatrix->process(this);

    for (auto module = _modules.begin(); module != _modules.end(); ++module) {
        (*module)->process(&_processBuffer, steadyTime);
        _processBuffer.swapInOut();
    }
    _processBuffer.swapInOut();
}

void Track::render() {
    for (auto module = _modules.begin(); module != _modules.end(); ++module) {
        ImGui::PushID((*module).get());
        (*module)->render();
        ImGui::PopID();
    }
    if (ImGui::Button("+")) {
        _openModuleSelector = true;
    }
    if (_openModuleSelector) {
        _composer->_pluginManager.openModuleSelector(this);
    }
}

void Track::addModule(std::string path, uint32_t index) {
    PluginHost* pluginHost = new PluginHost(this);
    pluginHost->load(path.c_str(), index);
    Module* module = new PluginModule(pluginHost->_name, this, pluginHost);
    _composer->_commandManager.executeCommand(new AddModuleCommand(this, module));
}
