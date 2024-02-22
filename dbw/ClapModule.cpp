#include "imgui.h"
#include "ClapModule.h"
#include "ClapHost.h"
#include "Track.h"
#include "Composer.h"
#include "Config.h"
#include "AudioEngine.h"

ClapModule::ClapModule(const nlohmann::json& json) : Module(json) {
    // TODO
}

ClapModule::ClapModule(std::string name, Track* track, ClapHost* pluginHost) : Module(name, track), _pluginHost(pluginHost) {
}

ClapModule::~ClapModule() {
    _pluginHost->closeGui();
    _pluginHost->stop();
    _pluginHost->unload();
}

bool ClapModule::process(ProcessBuffer* buffer, int64_t steadyTime) {
    _pluginHost->process(buffer, steadyTime);
    return Module::process(buffer, steadyTime);
}

tinyxml2::XMLElement* ClapModule::toXml(tinyxml2::XMLDocument* doc) {
    auto* element = doc->NewElement("ClapPlugin");
    element->SetAttribute("id", nekoId());
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

void ClapModule::openGui() {
    _pluginHost->openGui();
    Module::openGui();
}

void ClapModule::closeGui() {
    _pluginHost->closeGui();
    Module::closeGui();
}

void ClapModule::renderContent() {
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

void ClapModule::start() {
    _pluginHost->start(
        gPreference.sampleRate,
        gPreference.bufferSize
    );
    Module::start();
}

void ClapModule::stop() {
    Module::stop();
    _pluginHost->stop();
}
