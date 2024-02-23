#include "CommandWindow.h"
#include <map>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include "App.h"
#include "Composer.h"
#include "Project.h"

static std::map<std::string, std::function<void(Composer*)>> table = {
    {"Audio Setup", [](Composer* composer) { composer->app()->showAudioSetupWindow(); }},
    {"Save", [](Composer* composer) { composer->_project->save(); }},
    {"Scan Plugin", [](Composer* composer) { gPluginManager.scan(); }},
};

CommandWindow::CommandWindow(Composer* composer) : _composer(composer) {
}

void CommandWindow::render() {
    if (_show) {
        ImGui::OpenPopup("Command");
    }

    ImGui::SetNextWindowSizeConstraints(ImVec2(300, 200), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::BeginPopupModal("Command", &_show)) {

        if (ImGui::IsWindowAppearing()) {
            ImGui::SetKeyboardFocusHere();
        }
        bool run = false;
        if (ImGui::InputText("##query", &_query, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue)) {
            run = true;
        }
        for (const auto& [name, fun] : table) {
            auto q = _query.begin();
            for (auto c = name.begin(); c != name.end() && q != _query.end(); ++c) {
                if (std::tolower(*q) == std::tolower(*c)) {
                    ++q;
                }
            }
            if (q == _query.end()) {
                if (run) {
                    fun(_composer);
                    _show = false;
                } else {
                    if (ImGui::Button(name.c_str())) {
                        fun(_composer);
                        _show = false;
                    }
                    ImGui::SameLine();
                }
            }
            if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                _show = false;
            }
        }
        ImGui::EndPopup();
    }

    if (!_show && ImGui::IsKeyPressed(ImGuiKey_Semicolon)) {
        _show = true;
    }
}
