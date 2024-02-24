#pragma once
#include "BaseWindow.h"

class Composer;

class RackWindow : public BaseWindow {
public:
    RackWindow(Composer* composer);
    void render();

private:
    Composer* _composer;
     
};

