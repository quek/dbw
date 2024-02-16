#pragma once
#include "Clip.h"

class Clip;
class Lane;
class PianoRollWindow;
class Track;
class Lane;

class ClipSlot {
public:
    ClipSlot();
    void render(PianoRollWindow*);
    void play();
    void stop();

    std::unique_ptr<Clip> _clip;
    bool _playing = false;
};
