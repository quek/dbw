#pragma once
#include <vector>
#include <memory>
#include "Clip.h"
#include "Nameable.h"

class Scene;

class Lane : public Nameable {
public:
    Lane(Scene* scene);
    Scene* _scene;
    std::vector<std::unique_ptr<Clip>> _clips;
};
