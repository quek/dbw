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
    _pluginHost->process(buffer, steadyTime);
    return Module::process(buffer, steadyTime);
}

tinyxml2::XMLElement* PluginModule::toXml(tinyxml2::XMLDocument* doc) {
    auto* element = doc->NewElement("ClapPlugin");
    element->SetAttribute("id", xmlId());
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
        auto* state = element->InsertNewChildElement("State");
        state->SetAttribute("path", _pluginHost->_statePath.string().c_str());
        _pluginHost->saveState();
    }

    return element;
}

void PluginModule::openGui() {
    _pluginHost->openGui();
    Module::openGui();
}

void PluginModule::closeGui() {
    _pluginHost->closeGui();
    Module::closeGui();
}

void PluginModule::renderContent() {
    if (_didOpenGui) {
        if (ImGui::Button("Close")) {
            closeGui();
        }
    } else {
        if (ImGui::Button("Open")) {
            openGui();
        }
    }
}

void PluginModule::start() {
    _pluginHost->start(
        _track->_composer->_audioEngine->_sampleRate,
        _track->_composer->_audioEngine->_bufferSize
    );
    Module::start();
}

void PluginModule::stop() {
    _pluginHost->stop();
    Module::stop();
}
