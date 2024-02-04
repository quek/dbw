#pragma once

class Clip;
class Lane;
class Track;

class ClipSlot {
public:
    ClipSlot(Track* track, Lane* lane, Clip* clip = nullptr);

    Track* _track;
    Lane* _lane;
    Clip* _clip;
};
