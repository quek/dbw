#pragma once
#include <map>
#include "BaseWindow.h"

class Composer;
class Track;

class RackWindow : public BaseWindow {
public:
    RackWindow(Composer* composer);
    void render();

private:
    void renderHeader();
    void renderModules();
    void renderFaders();

    Composer* _composer;
    float _headerHeight = 0.0f;
    float _faderHeight = 150.0f;
};

