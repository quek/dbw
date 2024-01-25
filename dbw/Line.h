#pragma once
#include <string>

class Track; 

class Line
{
public:
    Line(Track* track);
    Line(const char* note, unsigned char velocity, unsigned char delay, Track* track);
    void render();

    Track* _track;
    std::string _note;
    std::string _lastNote;
    unsigned char _velocity = 0x64;
    unsigned char _lastVelocity = 0x64;
    bool _velocityEditing = false;
    unsigned char _delay = 0;
    unsigned char _lastDelay = 0;
};

