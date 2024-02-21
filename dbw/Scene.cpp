#include "Scene.h"
#include "ClipSlot.h"
#include "Composer.h"
#include "SceneMatrix.h"

Scene::Scene(SceneMatrix* sceneMatrix) : _sceneMatrix(sceneMatrix) {
    _name = "Scene" + std::to_string(_sceneMatrix->_scenes.size() + 1);
}

void Scene::play() {
    for (auto& [_, clipSlot] : _clipSlotMap) {
        clipSlot->play();
    }
    _sceneMatrix->_composer->play();
}

void Scene::stop() {
    for (auto& [_, clipSlot] : _clipSlotMap) {
        clipSlot->stop();
    }
}

bool Scene::isAllLanePlaying() {
    return false;
}

bool Scene::isAllLaneStoped() {
    return false;
}

std::unique_ptr<ClipSlot>& Scene::getClipSlot(Lane* lane) {
    auto x = _clipSlotMap.find(lane);
    if (x != _clipSlotMap.end()) {
        return x->second;
    }
    _clipSlotMap[lane] = std::make_unique<ClipSlot>();
    return _clipSlotMap[lane];
}

