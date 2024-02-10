#pragma once
#include <memory>
#include "Sequence.h"
#include "Thing.h"

class PianoRoll;

class Clip : public Nameable, public Thing {
public:
    Clip(double time = 0.0, double duration = 16.0);
    Clip(std::shared_ptr<Sequence> sequence);
    virtual ~Clip() = default;

    void renderInScene(PianoRoll* pianoRoll);

    std::shared_ptr<Sequence> _sequence;
};
