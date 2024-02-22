#pragma once
#include <map>
#include <memory>
#include "Nameable.h"

class ClipSlot;
class SceneMatrix;
class Lane;

class Scene : public Nameable {
public:
    Scene(SceneMatrix* sceneMatrix);
    void play();
    void stop();
    bool isAllLanePlaying();
    bool isAllLaneStoped();

private:
    SceneMatrix* _sceneMatrix;
};
