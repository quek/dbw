#pragma once
#include "Clip.h"

class Clip;
class Lane;
class Track;

class ClipSlot {
public:
    ClipSlot(Track* track, Lane* lane, Clip* clip = nullptr);
    void render();
    void play();
    void stop();

    Track* _track;
    Lane* _lane;
    std::unique_ptr<Clip> _clip;
    bool _playing = false;
};
