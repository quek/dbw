#pragma once
#include "Clip.h"

class Clip;
class Composer;

class ClipSlot {
public:
    ClipSlot();
    void render(Composer* composer);
    void play();
    void stop();

    std::unique_ptr<Clip> _clip;
    bool _playing = false;
};
