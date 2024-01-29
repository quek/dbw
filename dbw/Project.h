#pragma once
#include <filesystem>
#include "tinyxml2/tinyxml2.h"

class Composer;
class Track;

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
    void writeTrack(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* structure, Track* track, int index);
    void writeTrack(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* structure, Track* track, const char* id);
};

