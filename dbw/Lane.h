#pragma once
#include <memory>
#include <vector>
#include "ClipSlot.h"
#include "Nameable.h"

class Clip;
class Scene;

class Lane : Nameable {
public:
    inline static const char* TYPE = "lane";
    Lane() = default;
    Lane(const nlohmann::json& json);
    virtual ~Lane() = default;
    virtual nlohmann::json toJson() override;
    std::unique_ptr<ClipSlot>& getClipSlot(Scene* scene);

    std::vector<std::unique_ptr<Clip>> _clips;
    std::map<Scene*, std::unique_ptr<ClipSlot>> _sceneClipSlotMap;
    Track* _track = nullptr;
};
