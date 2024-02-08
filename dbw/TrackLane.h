#pragma once
#include <memory>
#include <vector>

class Clip;

class TrackLane {
public:
    std::vector<std::unique_ptr<Clip>> _clips;
};
