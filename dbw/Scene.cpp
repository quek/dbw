#include "Scene.h"
#include "Clip.h"
#include "ClipSlot.h"
#include "Composer.h"
#include "Lane.h"
#include "Track.h"
#include "SceneMatrix.h"

Scene::Scene(const nlohmann::json& json) : Nameable(json) {
}

Scene::Scene(SceneMatrix* sceneMatrix) : _sceneMatrix(sceneMatrix) {
    _name = "Scene" + std::to_string(_sceneMatrix->_scenes.size() + 1);
}

void Scene::play() {
    for (const auto& track : _sceneMatrix->_composer->getTracks()) {
        for (const auto& lane : track->_lanes) {
            lane->getClipSlot(this)->play();
        }
    }
    _sceneMatrix->_composer->play();
}

void Scene::stop() {
    for (const auto& track : _sceneMatrix->_composer->getTracks()) {
        for (const auto& lane : track->_lanes) {
            lane->getClipSlot(this)->stop();
        }
    }
}

bool Scene::isAllLanePlaying() {
    for (const auto& track : _sceneMatrix->_composer->getTracks()) {
        for (const auto& lane : track->_lanes) {
            if (!lane->getClipSlot(this)->_playing) {
                return false;
            }
        }
    }
    return true;
}

bool Scene::isAllLaneStoped() {
    for (const auto& track : _sceneMatrix->_composer->getTracks()) {
        for (const auto& lane : track->_lanes) {
            if (lane->getClipSlot(this)->_playing) {
                return false;
            }
        }
    }
    return true;
}

nlohmann::json Scene::toJson() {
    nlohmann::json json = Nameable::toJson();
    return json;
}

