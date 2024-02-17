#include "Track.h"
#include <mutex>
#include "imgui.h"
#include "AudioEngine.h"
#include "Clip.h"
#include "Command.h"
#include "Composer.h"
#include "Fader.h"
#include "Midi.h"
#include "Module.h"
#include "PluginModule.h"
#include "PluginHost.h"
#include "Lane.h"

Track::Track(std::string name, Composer* composer) :
    _fader(new Fader("Fader", this)), _name(name), _composer(composer){
    _trackLanes.emplace_back(new Lane());
    _fader->start();
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

    for (auto& module : _modules) {
        if (module->isStarting()) {
            module->process(&_processBuffer, steadyTime);
            _processBuffer.swapInOut();
        }
    }
    _fader->process(&_processBuffer, steadyTime);
}

void Track::render() {
    ImGui::PushID(this);
    for (auto& module : _modules) {
        module->render();
    }
    if (ImGui::Button("+")) {
        _openModuleSelector = true;
    }
    if (_openModuleSelector) {
        _composer->_pluginManager.openModuleSelector(this);
    }
    _fader->render();
    ImGui::PopID();
}

void Track::addModule(std::string path, uint32_t index) {
    PluginHost* pluginHost = new PluginHost(this);
    pluginHost->load(path.c_str(), index);
    Module* module = new PluginModule(pluginHost->_name, this, pluginHost);
    _composer->_commandManager.executeCommand(new AddModuleCommand(this, module));
}

bool Track::isAvailableSidechainSrc(Track* dst) {
    if (this == dst) {
        // 当該モジュールより前のは使える
        return true;
    }
    // TODO
    return true;
}

tinyxml2::XMLElement* Track::toXml(tinyxml2::XMLDocument* doc) {
    auto* trackElement = doc->NewElement("Track");
    trackElement->SetAttribute("id", xmlId());
    trackElement->SetAttribute("name", _name.c_str());

    // TODO _fader->toXml(doc)
    // <Mute value = "false" id = "id16" name = "Mute" / >
    // <Pan max = "1.000000" min = "0.000000" unit = "normalized" value = "0.500000" id = "id15" name = "Pan" / >
    // <Volume max = "2.000000" min = "0.000000" unit = "linear" value = "0.942701" id = "id14" name = "Volume" / >

    auto* channel = trackElement->InsertNewChildElement("Channel");
    channel->SetAttribute("role", role());
    auto* devices = channel->InsertNewChildElement("Devices");
    for (auto& module : _modules) {
        devices->InsertEndChild(module->dawProject(doc));
    }
    return trackElement;
}

std::unique_ptr<Track> Track::fromXml(tinyxml2::XMLElement* element) {
    // TODO
    return std::unique_ptr<Track>();
}
