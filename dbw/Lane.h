#pragma once
#include <vector>
#include <map>
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
    std::unique_ptr<ClipSlot>& getClipSlot(Track* track);

    Scene* _scene;
    std::map<Track*, std::unique_ptr<ClipSlot>> _clipSlots;
};
