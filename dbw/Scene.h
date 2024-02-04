#pragma once
#include <vector>
#include <memory>
#include "Lane.h"

class SceneMatrix;

class Scene : public Nameable {
public:
    Scene(SceneMatrix* sceneMatrix);
private:
    SceneMatrix* _sceneMatrix;
    std::vector<std::unique_ptr<Lane>> _lanes;
};
