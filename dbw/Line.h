#pragma once
#include <memory>
#include <string>
#include <vector>

class Track; 
class Column;

class Line
{
public:
    Line(Track* track, size_t ncolumns);
    Line(const char* note, unsigned char velocity, unsigned char delay, Track* track);
    void render();

    Track* _track;
    std::vector<std::unique_ptr<Column>> _columns;
};

