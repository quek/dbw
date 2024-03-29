#include "Project.h"
#include <fstream>
#include "Composer.h"
#include "Config.h"
#include "Error.h"
#include "FileDialog.h"
#include "SerializeContext.h"
#include "util.h"

Project::Project(Composer* composer) : _composer(composer)
{
}

Composer* Project::open(std::filesystem::path path)
{
    std::ifstream in(path);
    if (!in)
    {
        Error("{} を開けませんでした。", path.string());
    }
    nlohmann::json json;
    in >> json;


    SerializeContext context;
    Composer* composer = new Composer(json, context);
    composer->_project->_path = path;
    composer->_project->_isNew = false;

    return composer;
}

void Project::save()
{
    if (_isNew)
    {
        saveAs();
        return;
    }
    SerializeContext context;
    auto json = _composer->toJson(context);
    context.edit(json);
    std::ofstream out(_path);
    if (!out)
    {
        Error("{} を開けませんでした。", _path.string());
    }
    out << json.dump(2);
    out.close();
    if (!out)
    {
        Error("保存に失敗しました。{}", _path.string());
    }

    _isNew = false;
}

void Project::saveAs()
{
        std::filesystem::path defaultPath(gConfig.projectDir() / (AnsiStringToWideString(yyyyMmDd()) + L".json"));
        auto x = FileDialog::getSaveFileName(defaultPath);
        if (!x.first)
        {
            return;
        }
        _path = x.second;
        if (_path.extension() != L".json")
        {
            _path += L".json";
        }
        _isNew = false;
        save();
}

