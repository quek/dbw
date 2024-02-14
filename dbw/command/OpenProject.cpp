#include "OpenProject.h"
#include "../Composer.h"
#include "../Project.h"

command::OpenProject::OpenProject(const std::filesystem::path& path) : Command(false), _path(path) {
}

void command::OpenProject::execute(Composer* composer) {
    composer->_project->open(_path);
}
