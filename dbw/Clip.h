#pragma once
#include <memory>
#include "Sequence.h"
#include "Thing.h"

class PianoRollWindow;

class Clip : public Nameable, public Thing {
public:
    Clip(const Clip& other) = default;
    Clip(double time = 0.0, double duration = 16.0);
    Clip(std::shared_ptr<Sequence> sequence);
    virtual ~Clip() = default;

    std::string name();
    void renderInScene(PianoRollWindow* pianoRollWindow);

    std::shared_ptr<Sequence> _sequence;

    bool _selected = false;
};
