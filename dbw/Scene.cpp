#include "Scene.h"
#include "Clip.h"
#include "ClipSlot.h"
#include "Composer.h"
#include "Lane.h"
#include "Track.h"
#include "SceneMatrix.h"

Scene::Scene(const nlohmann::json& json, SerializeContext& context) : Nameable(json, context) {
}

Scene::Scene(SceneMatrix* sceneMatrix) : _sceneMatrix(sceneMatrix) {
    _name = "Scene" + std::to_string(_sceneMatrix->_scenes.size() + 1);
}

void Scene::play() {
    _sceneMatrix->_composer->_masterTrack->play(this);
    _sceneMatrix->_composer->play();
}

void Scene::stop() {
    _sceneMatrix->_composer->_masterTrack->stop(this);
}

bool Scene::isAllLanePlaying() {
    return _sceneMatrix->_composer->_masterTrack->isAllLanesPlaying(this);
}

bool Scene::isAllLaneStoped() {
    return _sceneMatrix->_composer->_masterTrack->isAllLanesStoped(this);
}

nlohmann::json Scene::toJson(SerializeContext& context) {
    nlohmann::json json = Nameable::toJson(context);
    return json;
}

