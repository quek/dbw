#pragma once
#include <memory>
#include "Sequence.h"

class Clip : public Nameable {
public:
    Clip();
    Clip(std::shared_ptr<Sequence> sequence);
    void renderInScene();

    std::shared_ptr<Sequence> _sequence;
};
