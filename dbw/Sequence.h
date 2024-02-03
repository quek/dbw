#pragma once
#include "Line.h"
#include "Nameable.h"

class Sequence : public Nameable {
private:
    int _ncolumns;
    int _nlines;
    std::vector<std::unique_ptr<Line>> _lines;
};
