#pragma once
#include <memory>
#include "Sequence.h"

class Clip : public Nameable {
public:
    void renderInScene();
    void play();

private:
    std::shared_ptr<Sequence> _sequences;
};
