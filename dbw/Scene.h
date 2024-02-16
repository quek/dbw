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

    std::unique_ptr<ClipSlot>& getClipSlot(Lane* lane);
    std::map<Lane*, std::unique_ptr<ClipSlot>> _clipSlotMap;

private:
    SceneMatrix* _sceneMatrix;
};
