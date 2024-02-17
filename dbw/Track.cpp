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
    _fader(new Fader("Fader", this)), _name(name), _composer(composer) {
    _lanes.emplace_back(new Lane());
    _fader->start();
}

Track::~Track() {
}

void Track::process(int64_t steadyTime) {
    double oneBeatSec = 60.0 / _composer->_bpm;
    double sampleRate = _composer->_audioEngine->_sampleRate;
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

    auto channel = trackElement->InsertNewChildElement("Channel");
    channel->SetAttribute("role", role());

    auto mute = channel->InsertNewChildElement("Mute");
    mute->SetAttribute("value", _fader->_mute);
    auto pan = channel->InsertNewChildElement("Pan");
    pan->SetAttribute("value", _fader->_pan);
    auto volume = channel->InsertNewChildElement("Volume");
    volume->SetAttribute("value", _fader->_level);

    auto* devices = channel->InsertNewChildElement("Devices");
    for (auto& module : _modules) {
        devices->InsertEndChild(module->toXml(doc));
    }
    return trackElement;
}

std::unique_ptr<Track> Track::fromXml(tinyxml2::XMLElement* element, Composer* composer) {
    auto name = element->Attribute("name");
    std::unique_ptr<Track> track(new Track(name, composer));

    auto channelElement = element->FirstChildElement("Channel");
    auto mute = channelElement->FirstChildElement("Mute");
    bool boolValue;
    mute->QueryBoolAttribute("value", &boolValue);
    track->_fader->_mute = boolValue;
    auto pan = channelElement->FirstChildElement("Pan");
    float floatValue;
    pan->QueryFloatAttribute("value", &floatValue);
    track->_fader->_pan = floatValue;
    auto volume = channelElement->FirstChildElement("Volume");
    volume->QueryFloatAttribute("value", &floatValue);
    track->_fader->_level = floatValue;

    for (auto deviceElement = channelElement->FirstChildElement("Devices")->FirstChildElement();
         deviceElement != nullptr;
         deviceElement = deviceElement->NextSiblingElement()) {
        if (deviceElement) {
            // TODO fromXML 
            Module* module = composer->_pluginManager.create(deviceElement, track.get());
            if (module != nullptr) {
                track->_modules.push_back(std::unique_ptr<Module>(module));
                module->start();
            }
        }
    }
    return track;
}
