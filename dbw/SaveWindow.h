#pragma once
#include <string>

class Composer;

class SaveWindow {
public:
    SaveWindow(Composer* composer);
    void render();
private:
    Composer* _composer;
    std::string _projectDir;
    std::string _projectName;
};
