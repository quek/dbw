#pragma once
#include <memory>
#include "Sequence.h"

class PianoRoll;

class Clip : public Nameable {
public:
    Clip(double time = 0.0, double duration = 16.0);
    Clip(std::shared_ptr<Sequence> sequence);
    void renderInScene(PianoRoll* pianoRoll);

    std::shared_ptr<Sequence> _sequence;
    double _time;
    double _duration;
};
