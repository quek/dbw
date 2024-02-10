#pragma once

class Grid;

class GridMixin {
public:
    GridMixin();
    virtual ~GridMixin() = default;
    void renderGridSnap();

    Grid* _grid;
    bool _snap = true;
};
