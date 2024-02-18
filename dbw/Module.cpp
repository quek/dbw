#include "Module.h"
#include <mutex>
#include "imgui.h"
#include "AudioEngine.h"
#include "Composer.h"
#include "Command.h"
#include "Track.h"

Module::~Module() {
    closeGui();
    stop();
}

void Module::start() {
    _track->_processBuffer.ensure(_track->_composer->_audioEngine->_bufferSize, std::max(_ninputs, _noutputs));
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
                    _track->_composer->_commandManager.executeCommand(new DeleteModuleCommand(this));
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

bool Module::isWaitingFrom() {
    for (auto& connection : _connections) {
        if (connection->_to == this && !connection->_from->_processed && connection->_from->isStarting()) {
            return true;
        }

    }
    return false;
}

bool Module::isWaitingTo() {
    for (auto& connection : _connections) {
        if (connection->_from == this && !connection->_to->_processed && connection->_to->isStarting()) {
            return true;
        }
    }
    return false;
}

bool Module::process(ProcessBuffer* /*buffer*/, int64_t /*steadyTime*/) {
    _processed = true;
    return true;
}

void Module::processConnections() {
    for (auto& connection : _connections) {
        if (connection->_to != this) {
            continue;
        }
        if (!connection->_from->isStarting()) {

        }
        connection->_from->_track->_processBuffer._out;

    }
}

void Module::prepare() {
    _processed = false;
}

void Module::connect(Module* from, int outputIndex, int inputIndex) {
    _connections.emplace_back(new Connection(from, outputIndex, this, inputIndex));
    from->_connections.emplace_back(new Connection(from, outputIndex, this, inputIndex));
}

ProcessBuffer& Module::getProcessBuffer() {
    return _track->_processBuffer;
}

tinyxml2::XMLElement* Module::toXml(tinyxml2::XMLDocument* doc) {
    auto element = doc->NewElement("Connections");
    for (auto& connection : _connections) {
        element->InsertEndChild(connection->toXml(doc));
    }
    return element;
}
