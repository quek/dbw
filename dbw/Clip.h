#pragma once
#include <memory>
#include "Sequence.h"

class Clip : public Nameable {
private:
    std::shared_ptr<Sequence> _sequences;
};
