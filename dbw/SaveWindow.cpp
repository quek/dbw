#include "SaveWindow.h"
#include <imgui.h>
#include "misc/cpp/imgui_stdlib.h"
#include "../ImGuiFileDialog/ImGuiFileDialog.h"
#include "Composer.h"
#include "ComposerWindow.h"
#include "Project.h"
#include "util.h"

SaveWindow::SaveWindow(Composer* composer) :
    _composer(composer),
    _projectName(composer->_project->_name.string()),
    _projectDir(composer->_project->_dir.string()) {
}

void SaveWindow::render() {
    if (!_composer->_composerWindow->_showSaveWindow) {
        return;
    }
    if (ImGui::Begin("Save", &_composer->_composerWindow->_showSaveWindow)) {
        ImGui::Text(_projectDir.c_str());
        ImGui::SameLine();
        if (ImGui::Button("Change##dir")) {
            IGFD::FileDialogConfig config;
            config.path = _projectDir;
            config.flags = ImGuiFileDialogFlags_Modal;
            ImGuiFileDialog::Instance()->OpenDialog("Project dir", "Choose File", nullptr, config);
        }

        ImGui::InputText("##Project name", &_projectName);

        auto path = std::filesystem::path(_projectDir) / std::filesystem::path(_projectName);
        bool exists = std::filesystem::exists(path);
        if (exists) {
            ImGui::Text("Already exists.");
        }
        ImGui::BeginDisabled(exists);
        if (ImGui::Button("Save")) {
            _composer->_composerWindow->_showSaveWindow = false;
            save();
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            _composer->_composerWindow->_showSaveWindow = false;
        }

        if (ImGuiFileDialog::Instance()->Display("Project dir")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                _projectDir = ImGuiFileDialog::Instance()->GetCurrentPath();
            }
            ImGuiFileDialog::Instance()->Close();
        }
    }
    ImGui::End();
}

void SaveWindow::save() {
    Project* project = _composer->_project.get();
    project->_name = std::filesystem::path(_projectName);
    project->_dir = std::filesystem::path(_projectDir);
    project->save();
    _composer->_composerWindow->setStatusMessage(std::string("Project is saved ") + yyyyMmDdHhMmSs());
}
