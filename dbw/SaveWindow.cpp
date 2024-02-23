#include "SaveWindow.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "misc/cpp/imgui_stdlib.h"
#include "../ImGuiFileDialog/ImGuiFileDialog.h"
#include "Composer.h"
#include "ComposerWindow.h"
#include "Config.h"
#include "Project.h"
#include "util.h"

SaveWindow::SaveWindow(Composer* composer) :
    _composer(composer) {
    if (composer->_project->_isNew) {
        _path = (gConfig.projectDir() / (yyyyMmDd() + ".json")).string();
    } else {
        _path = composer->_project->_path.string();
    }
}

void SaveWindow::render() {
    if (!_composer->_composerWindow->_showSaveWindow) {
        return;
    }
    if (ImGui::Begin("Save", &_composer->_composerWindow->_showSaveWindow)) {
        ImGui::Text(_path.c_str());
        ImGui::SameLine();
        if (ImGui::Button("Change##path")) {
            IGFD::FileDialogConfig config;
            config.path = _path;
            config.flags = ImGuiFileDialogFlags_Modal;
            ImGuiFileDialog::Instance()->OpenDialog("Project dir", "Choose File", ".json", config);
        }

        bool exists = std::filesystem::exists(_path);
        if (exists) {
            ImGui::Text("Already exists!!!");
            ImGui::Text("Already exists!!!");
            ImGui::Text("Already exists!!!");
        }
        if (ImGui::Button("Save")) {
            _composer->_composerWindow->_showSaveWindow = false;
            save();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            _composer->_composerWindow->_showSaveWindow = false;
        }

        if (ImGuiFileDialog::Instance()->Display("Project dir")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                _path = ImGuiFileDialog::Instance()->GetFilePathName();
            }
            ImGuiFileDialog::Instance()->Close();
        }
    }
    ImGui::End();
}

void SaveWindow::save() {
    Project* project = _composer->_project.get();
    project->_path = _path;
    project->save();
    _composer->_composerWindow->setStatusMessage(std::string("Project is saved ") + yyyyMmDdHhMmSs());
}
