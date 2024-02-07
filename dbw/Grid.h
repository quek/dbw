#pragma once
#include <memory>
#include <string>
#include <vector>

class Grid {
public:
    Grid(std::string name, double unit);
    std::string _name;
    double _unit;

    double snapFloor(double time);
    double snapRound(double time);

    static void init();
};

extern std::vector<std::unique_ptr<Grid>> gGrids;
