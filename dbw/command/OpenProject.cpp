#include "OpenProject.h"
#include "../App.h"
#include "../Composer.h"
#include "../Config.h"
#include "../FileDialog.h"
#include "../Project.h"
#include "../util.h"

command::OpenProject::OpenProject() : Command(false), _path("")
{
}

command::OpenProject::OpenProject(const std::filesystem::path& path) : Command(false), _path(path) {
}

void command::OpenProject::execute(Composer* composer) {
    if (_path.empty())
    {
        auto x = FileDialog::getOpenFileName(gConfig.projectDir(), { {L"JSON(*.json)", L"*.json"}, { L"All(*.*)", L"*.*" } });
        if (!x.first)
        {
            return;
        }
        _path = x.second;
    }
    Composer* newComposer = composer->_project->open(_path);
    composer->app()->requestAddComposer(newComposer);
    composer->app()->requestDeleteComposer(composer);
}
