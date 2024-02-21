#include "Module.h"
#include <mutex>
#include "imgui.h"
#include "AudioEngine.h"
#include "BuiltinModule.h"
#include "PluginModule.h"
#include "Composer.h"
#include "Command.h"
#include "Config.h"
#include "Track.h"
#include "Vst3Module.h"
#include "command/DeleteModule.h"

Module::~Module() {
    closeGui();
    stop();
}

void Module::start() {
    _track->_processBuffer.ensure(gPreference.bufferSize, max(_ninputs, _noutputs), 2);
    _isStarting = true;
}

void Module::render() {
    ImGui::PushID(this);
    ImGuiChildFlags childFlags = ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY;
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar;
    if (ImGui::BeginChild("##module", ImVec2(0, 0), childFlags, windowFlags)) {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu(_name.c_str())) {
                if (ImGui::MenuItem("Delete")) {
                    _track->_composer->_commandManager.executeCommand(new command::DeleteModule(this));
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        if (_ninputs > 1) {
            if (ImGui::Button("Sidechain")) {
                // TODO inputIndex
                _track->_composer->_sideChainInputSelector->open(this, 1);
            }
        }

        // TODO とりあえずいまは表示だけ
        for (auto& c : _connections) {
            auto x = c->_from->_name + " " + std::to_string(c->_fromIndex) + " ⇒ " + c->_to->_name + " " + std::to_string(c->_toIndex);
            ImGui::Text(x.c_str());
        }

        renderContent();
    }
    ImGui::EndChild();
    ImGui::PopID();
}

bool Module::process(ProcessBuffer* /*buffer*/, int64_t /*steadyTime*/) {
    return true;
}

void Module::processConnections() {
    for (auto& connection : _connections) {
        connection->process(this);
    }
}

void Module::connect(Module* from, int outputIndex, int inputIndex) {
    _connections.emplace_back(new Connection(from, outputIndex, this, inputIndex));
    from->_connections.emplace_back(new Connection(from, outputIndex, this, inputIndex));
}

int Module::nbuses() const {
    return max(_ninputs, _noutputs);
}

ProcessBuffer& Module::getProcessBuffer() {
    return _track->_processBuffer;
}

uint32_t Module::getComputedLatency() {
    return _computedLatency;
}

void Module::setComputedLatency(uint32_t computedLatency) {
    _computedLatency = computedLatency;
    for (auto& connection : _connections) {
        if (connection->_to == this) {
            if (connection->_from->getComputedLatency() > computedLatency) {
                connection->setLatency(connection->_from->getComputedLatency() - computedLatency);
            } else {
                connection->setLatency(0);
            }
        }
    }
}

tinyxml2::XMLElement* Module::toXml(tinyxml2::XMLDocument* doc) {
    auto element = doc->NewElement("Connections");
    for (auto& connection : _connections) {
        element->InsertEndChild(connection->toXml(doc));
    }
    return element;
}

Module* Module::create(std::string& type, std::string& id) {
    if (type == "builtin") {
        return BuiltinModule::create(id);
    } else if (type == "vst3") {
        return Vst3Module::create(id);
    } else if (type == "clap") {
        //return ClapModule::create(id);
    }
    return nullptr;
}

Module* Module::fromJson(const nlohmann::json& json) {
    auto& type = json["type"];
    if (type == "builtin") {
        //return BuiltinModule::fromJson(json);
    } else if (type == "vst3") {
        return Vst3Module::fromJson(json);
    } else if (type == "clap") {
        //return PluginModule::fromJson(json);
    }
    return nullptr;
}
