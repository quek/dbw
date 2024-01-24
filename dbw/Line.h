#pragma once
#include <string>

class Line
{
public:
    void render();

    std::string _note;
    int _velocity = 0x64;
    int _delay;
};

