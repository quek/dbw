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
    if (ImGui::BeginChild("##window", ImVec2(0, 0), ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_MenuBar)) {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu(_name.c_str())) {
                if (ImGui::MenuItem("Delete")) {
                    _track->_composer->_commandManager.executeCommand(new DeleteModuleCommand(this));
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        renderContent();
    }
    ImGui::EndChild();
    ImGui::PopID();
}
