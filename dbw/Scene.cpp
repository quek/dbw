#include "Scene.h"
#include "SceneMatrix.h"

Scene::Scene(SceneMatrix* sceneMatrix) : _sceneMatrix(sceneMatrix) {
    _lanes.emplace_back(new Lane(this));
    _name = "Scene" + std::to_string(_sceneMatrix->_scenes.size() + 1);
}
