#pragma once
#include <string>

class Line; 

class Column
{
public:
    Column(Line* line);
    Column(const char* note, unsigned char velocity, unsigned char delay, Line* track);
    void render();

    Line* _line;
    std::string _note;
    std::string _lastNote;
    bool _noteEditing = false;
    unsigned char _velocity = 0x64;
    unsigned char _lastVelocity = 0x64;
    bool _velocityEditing = false;
    unsigned char _delay = 0;
    unsigned char _lastDelay = 0;
    bool _delayEditing = false;
};

