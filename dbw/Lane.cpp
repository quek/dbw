#include "Lane.h"
#include "Clip.h"
#include "Composer.h"
#include "SceneMatrix.h"
#include "Track.h"

Lane::Lane(const nlohmann::json& json) : Nameable(json) {
    for (const auto& x : json["_clips"]) {
        _clips.emplace_back(Clip::create(x));
    }

    for (const auto& x : json["_sceneClipSlotMap"].items()) {
        Scene* scene = Neko::findByNekoId<Scene>(std::stoull(x.key()));
        if (scene) {
            _sceneClipSlotMap[scene] = std::make_unique<ClipSlot>(x.value());
        }
    }

    if (json.contains("_automationTarget")) {
        _automationTarget.reset(new AutomationTarget(json["_automationTarget"]));
    }
}

nlohmann::json Lane::toJson() {
    nlohmann::json json = Nameable::toJson();
    json["type"] = TYPE;
    nlohmann::json clips = nlohmann::json::array();
    for (const auto& clip : _clips) {
        clips.emplace_back(clip->toJson());
    }
    json["_clips"] = clips;

    nlohmann::json map({});
    for (const auto& scene : _track->getComposer()->_sceneMatrix->_scenes) {
        auto& clipSlot = getClipSlot(scene.get());
        map[std::to_string(scene->getNekoId())] = clipSlot->toJson();
    }
    json["_sceneClipSlotMap"] = map;

    if (_automationTarget) {
        json["_automationTarget"] = _automationTarget->toJson();
    }


    return json;
}

void Lane::prepareProcessBuffer(double begin, double end, double loopBegin, double loopEnd, double oneBeatSec) {
    for (auto& clip : _clips) {
        clip->prepareProcessBuffer(this, begin, end, loopBegin, loopEnd, oneBeatSec);
    }
}

std::unique_ptr<ClipSlot>& Lane::getClipSlot(Scene* scene) {
    auto x = _sceneClipSlotMap.find(scene);
    if (x != _sceneClipSlotMap.end()) {
        return x->second;
    }
    _sceneClipSlotMap[scene] = std::make_unique<ClipSlot>();
    return _sceneClipSlotMap[scene];
}
