#pragma once
#include <memory>
#include "Sequence.h"

class Clip : public Nameable {
public:
    void renderInScene();

private:
    std::shared_ptr<Sequence> _sequences;
};
