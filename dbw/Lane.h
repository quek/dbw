#pragma once
#include <vector>
#include <map>
#include <memory>
#include "ClipSlot.h"
#include "Nameable.h"

class Clip;
class Track;

class Lane : public Nameable {
public:
    Lane();
    void render(Track* track);
    std::unique_ptr<ClipSlot>& getClipSlot(Track* track);

    std::map<Track*, std::unique_ptr<ClipSlot>> _clipSlots;
};
