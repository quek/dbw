#pragma once
#include <cstdint>
#include "Thing.h"

class Note : public Thing {
public:
    Note(const Note& other) = default;
    Note(double time = 0.0f, double duration = 1.0f, int16_t key = 64, double velocity = 0.8, int16_t channel = 0);
    virtual ~Note() = default;
    int16_t _channel;
    int16_t _key;
    double _velocity;
    double _rel;
};
