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
    void open();
    void save();
    std::filesystem::path projectDir() const;
    std::filesystem::path projectXml() const;
    Composer* _composer;

    bool _isNew = true;
    std::filesystem::path _dir;
    std::filesystem::path _name;

private:
    void writeTrack(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* structure, Track* track, const char* role = "regular");
    bool contaisId(void* x);
    std::string generateId(void* x, std::string prefix);
    std::string getId(void* x);
    void setId(void* x, std::string id);
    void* getObject(std::string id);

    std::map<void*, std::string> _idMap;
    std::map<std::string, void*> _idMapRev;
};
