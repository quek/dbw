#pragma once
#include <cstdint>

class Note {
public:
    double _time;
    double _duration;
    int16_t _channel;
    int16_t _key;
    float _velocity;
};
