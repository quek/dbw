#include "Lane.h"
#include "Clip.h"
#include "Composer.h"
#include "SceneMatrix.h"
#include "Track.h"

Lane::Lane(const nlohmann::json& json) : Nameable(json) {
    for (const auto& x : json["_clips"]) {
        _clips.emplace_back(new Clip(x));
    }

    for (const auto& x : json["_sceneClipSlotMap"].items()) {
        Scene* scene = Neko::findByNekoId<Scene>(std::stoull(x.key()));
        if (scene) {
            _sceneClipSlotMap[scene] = std::make_unique<ClipSlot>(x.value());
        }
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
    for (const auto& scene : _track->_composer->_sceneMatrix->_scenes) {
        auto& clipSlot = getClipSlot(scene.get());
        map[std::to_string(scene->nekoId())] = clipSlot->toJson();
    }
    json["_sceneClipSlotMap"] = map;


    return json;
}

std::unique_ptr<ClipSlot>& Lane::getClipSlot(Scene* scene) {
    auto x = _sceneClipSlotMap.find(scene);
    if (x != _sceneClipSlotMap.end()) {
        return x->second;
    }
    _sceneClipSlotMap[scene] = std::make_unique<ClipSlot>();
    return _sceneClipSlotMap[scene];
}
