#pragma once
#include <memory>
#include <vector>

class Clip;

class Lane {
public:
    std::vector<std::unique_ptr<Clip>> _clips;
};
