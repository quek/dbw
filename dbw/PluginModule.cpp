#include "imgui.h"
#include "PluginModule.h"
#include "PluginHost.h"
#include "Track.h"
#include "Composer.h"
#include "AudioEngine.h"

PluginModule::PluginModule(std::string name, Track* track, PluginHost* pluginHost) : Module(name, track), _pluginHost(pluginHost) {
}

PluginModule::~PluginModule() {
    _pluginHost->closeGui();
    _pluginHost->stop();
    _pluginHost->unload();
}

void PluginModule::process(ProcessBuffer* in, unsigned long framesPerBuffer, int64_t steadyTime) {
    Module::process(in, framesPerBuffer, steadyTime);
    auto process = _pluginHost->process(in, framesPerBuffer, steadyTime);

    auto buffer = process->audio_outputs;
    bool isLeftConstant = (buffer->constant_mask & (static_cast<uint64_t>(1) << 0)) != 0;
    bool isRightConstant = (buffer->constant_mask & (static_cast<uint64_t>(1) << 1)) != 0;
    for (unsigned long i = 0; i < framesPerBuffer; ++i) {
        _processBuffer._out[0][i] = buffer->data32[0][isLeftConstant ? 0 : i];
        _processBuffer._out[1][i] = buffer->data32[1][isRightConstant ? 0 : i];
    }
}

tinyxml2::XMLElement* PluginModule::dawProject(tinyxml2::XMLDocument* doc) {
    auto* element = doc->NewElement("ClapPlugin");
    // TODO Possible values: instrument, noteFX, audioFX, analyzer
    element->SetAttribute("deviceRole", "instrument");
    element->SetAttribute("deviceName", _name.c_str());
    element->SetAttribute("deviceID", _pluginHost->_plugin->desc->id);
    {
        element->InsertNewChildElement("Parameters");
    }
    {
        auto* enabled = element->InsertNewChildElement("Enabled");
        enabled->SetAttribute("value", true);
    }
    {
        auto* state= element->InsertNewChildElement("State");
        state->SetAttribute("path", _pluginHost->_statePath.string().c_str());
        _pluginHost->saveState();
    }

    return element;
}

void PluginModule::openGui() {
    _pluginHost->openGui();
    Module::openGui();
};

void PluginModule::closeGui() {
    _pluginHost->closeGui();
    Module::closeGui();
};

void PluginModule::start() {
    _pluginHost->start(
        _track->_composer->_audioEngine->_sampleRate,
        _track->_composer->_audioEngine->_bufferSize
    );
};

void PluginModule::stop() {
    _pluginHost->stop();
};
