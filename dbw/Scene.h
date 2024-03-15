#pragma once
#include <map>
#include <memory>
#include "Nameable.h"

class ClipSlot;
class SceneMatrix;
class Lane;

class Scene : public Nameable {
public:
    Scene(const nlohmann::json& json, SerializeContext& context);
    Scene(SceneMatrix* sceneMatrix);
    void play();
    void stop();
    bool isAllLanePlaying();
    bool isAllLaneStoped();
    virtual nlohmann::json toJson(SerializeContext& context) override;

    SceneMatrix* _sceneMatrix = nullptr;
};
