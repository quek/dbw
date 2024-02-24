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
     
    Composer* _composer;
};

