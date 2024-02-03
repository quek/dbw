#pragma once
#include <vector>
#include <memory>
#include "Clip.h"

class Track;

class Scene : public Nameable {
public:
    Scene(Track* track);
private:
    Track* _track;
    std::vector<std::unique_ptr<Clip>> _clips;
};
