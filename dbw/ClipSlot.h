#pragma once
#include "Clip.h"

class Composer;

class ClipSlot : Nameable{
public:
    ClipSlot();
    ClipSlot(const nlohmann::json& json, SerializeContext& context);
    void render(Composer* composer);
    void play();
    void stop();
    nlohmann::json toJson(SerializeContext& context) override;

    std::unique_ptr<Clip> _clip;
    bool _playing = false;
};
