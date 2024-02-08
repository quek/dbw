#pragma once

class Grid;

class GridMixin {
public:
    GridMixin();
    virtual ~GridMixin() = default;
    void renderGridSnap();
protected:
    Grid* _grid;
    bool _snap = true;
};
