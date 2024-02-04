#pragma once
#include <memory>
#include <vector>
#include "Nameable.h"
#include "Note.h"

class Sequence : public Nameable {
public:
    Sequence();

private:
    std::vector<std::unique_ptr<Note>> _notes;
};
