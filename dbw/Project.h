#pragma once
#include <filesystem>
#include <map>
#include <string>
#include "tinyxml2/tinyxml2.h"

class Composer;
class Track;

class Project {
public:
    Project(std::string name, Composer* composer);
    void open(std::filesystem::path dir);
    void save();
    std::filesystem::path projectDir() const;
    std::filesystem::path projectXml() const;
    Composer* _composer;

    bool _isNew = true;
    std::filesystem::path _dir;
    std::filesystem::path _name;

private:
    std::map<void*, std::string> _idMap;
    std::map<std::string, void*> _idMapRev;
};
