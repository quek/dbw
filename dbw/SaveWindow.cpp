#include "SaveWindow.h"
#include <imgui.h>
#include "misc/cpp/imgui_stdlib.h"
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
        ImGui::InputText("Project dir", &_projectDir);
        ImGui::InputText("Project name", &_projectName);
        if (ImGui::Button("Save")) {
            _composer->_project->_name = std::filesystem::path(_projectName);
            _composer->_project->_dir = std::filesystem::path(_projectDir);
            _composer->_project->save();
            _composer->_composerWindow->setStatusMessage(std::string("Project is saved ") + yyyyMmDdHhMmSs());
            _composer->_composerWindow->_showSaveWindow = false;
        }
    }
    ImGui::End();
}
