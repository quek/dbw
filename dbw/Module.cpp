#include "Module.h"
#include <mutex>
#include "imgui.h"
#include "AudioEngine.h"
#include "Composer.h"
#include "Command.h"

Module::~Module() {
    closeGui();
    stop();
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
            auto x = c->_from->_name + " " + std::to_string(c->_fromIndex) + " => " + c->_to->_name + " " + std::to_string(c->_toIndex);
            ImGui::Text(x.c_str());
        }

        renderContent();
    }
    ImGui::EndChild();
    ImGui::PopID();
}

void Module::connect(Module* from, int outputIndex, int inputIndex) {
    _connections.emplace_back(new Connection(from, outputIndex, this, inputIndex));
}

tinyxml2::XMLElement* Module::toXml(tinyxml2::XMLDocument* doc) {
    auto element = doc->NewElement("Connections");
    for (auto& connection : _connections) {
        element->InsertEndChild(connection->toXml(doc));
    }
    return element;
}
