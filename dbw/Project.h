#pragma once
#include <filesystem>

class Composer;

class Project {
public:
    Project(Composer* composer);
    Composer* open(std::filesystem::path dir);
    void save();

    Composer* _composer;
    bool _isNew = true;
    std::filesystem::path _path;
};
