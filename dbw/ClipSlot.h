#pragma once
#include "Clip.h"

class Composer;

class ClipSlot : Nameable{
public:
    ClipSlot();
    ClipSlot(const nlohmann::json& josn);
    void render(Composer* composer);
    void play();
    void stop();
    virtual nlohmann::json toJson() override;

    std::unique_ptr<Clip> _clip;
    bool _playing = false;
};
