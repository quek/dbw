#pragma once
#include <filesystem>

class Composer;

class Project {
public:
    Project(std::string name, Composer* composer);
    void open();
    void save();
    std::filesystem::path projectDir() const;
    std::filesystem::path projectXml() const;
    std::filesystem::path _dir;
    std::filesystem::path _name;
    Composer* _composer;
private:
};

