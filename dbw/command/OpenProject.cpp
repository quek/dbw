#include "OpenProject.h"
#include "../App.h"
#include "../Composer.h"
#include "../Project.h"

command::OpenProject::OpenProject(const std::filesystem::path& path) : Command(false), _path(path) {
}

void command::OpenProject::execute(Composer* composer) {
    Composer* newComposer = composer->_project->open(_path);
    composer->app()->requestAddComposer(newComposer);
    composer->app()->requestDeleteComposer(composer);
}
