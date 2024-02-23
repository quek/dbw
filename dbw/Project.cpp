#include "Project.h"
#include <fstream>
#include "Composer.h"
#include "Error.h"

Project::Project(Composer* composer) : _composer(composer) {
}

Composer* Project::open(std::filesystem::path path) {
    std::ifstream in(path);
    if (!in) {
        Error("{} を開けませんでした。", path.string());
    }
    nlohmann::json json;
    in >> json;


    Composer* composer = new Composer(json);
    composer->_project->_path = path;
    composer->_project->_isNew = false;

    return composer;
}

void Project::save() {
    auto json = _composer->toJson();
    std::ofstream out(_path);
    if (!out) {
        Error("{} を開けませんでした。", _path.string());
    }
    out << json.dump(2);
    out.close();
    if (!out) {
        Error("保存に失敗しました。{}", _path.string());
    }

    _isNew = false;
}

