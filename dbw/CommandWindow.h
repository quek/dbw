#pragma once
#include <string>

class Composer;

class CommandWindow {
public:
    CommandWindow(Composer* composer);
    void render();
private:
    bool _show = false;
    Composer* _composer;
    std::string _query;
};

