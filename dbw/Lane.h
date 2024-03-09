#pragma once
#include <memory>
#include <vector>
#include "AutomationTarget.h"
#include "ClipSlot.h"
#include "Nameable.h"

class Clip;
class Scene;

class Lane : public Nameable {
public:
    inline static const char* TYPE = "lane";
    Lane() = default;
    Lane(const nlohmann::json& json);
    virtual ~Lane() = default;
    virtual nlohmann::json toJson() override;
    std::unique_ptr<ClipSlot>& getClipSlot(Scene* scene);
    void prepareProcessBuffer(ProcessBuffer& processBuffer, double begin, double end, double loopBegin, double loopEnd, double oneBeatSec);


    std::vector<std::unique_ptr<Clip>> _clips;
    std::map<Scene*, std::unique_ptr<ClipSlot>> _sceneClipSlotMap;
    Track* _track = nullptr;
    std::unique_ptr<AutomationTarget> _automationTarget = nullptr;
};
