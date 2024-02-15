#pragma once

class Thing {
public:
    Thing(const Thing& other) = default;
    Thing(double time = 0.0f, double duration = 0.0f);
    virtual ~Thing() = default;

    double _time;
    double _duration;
};
