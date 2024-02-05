#include "Grid.h"

std::vector<std::unique_ptr<Grid>> gGrids;

Grid::Grid(std::string name, double unit) : _name(name), _unit(unit) {
}

void Grid::init() {
    gGrids.emplace_back(new Grid("Bar", 4));
    gGrids.emplace_back(new Grid("Beat", 1));
    gGrids.emplace_back(new Grid("1/8", 0.5));
    gGrids.emplace_back(new Grid("1/16", 0.25));
    gGrids.emplace_back(new Grid("1/32", 0.125));
    gGrids.emplace_back(new Grid("1/64", 0.0625));
}
