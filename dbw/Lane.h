#pragma once
#include <vector>
#include <memory>
#include "ClipSlot.h"
#include "Nameable.h"

class Clip;
class Scene;
class Track;

class Lane : public Nameable {
public:
    Lane(Scene* scene);
    void render(Track* track);
    Clip* findClip(Track* track);

    Scene* _scene;
    std::vector<std::unique_ptr<ClipSlot>> _clipSlots;
};
