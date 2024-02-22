#include "Lane.h"
#include "Clip.h"

Lane::Lane(const nlohmann::json& json) : Nameable(json) {
    for (const auto& x : json["_clips"]) {
        _clips.emplace_back(new Clip(x));
    }
}

nlohmann::json Lane::toJson() {
    nlohmann::json json = Nameable::toJson();
    json["type"] = TYPE;
    for (const auto& clip : _clips) {
        json["_clips"].push_back(clip->toJson());
    }
    return json;
}
