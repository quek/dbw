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

bool PluginModule::process(ProcessBuffer* buffer, int64_t steadyTime) {
    return _pluginHost->process(buffer, steadyTime);
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
