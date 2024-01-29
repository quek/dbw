#pragma once
#include "Track.h"

class Composer;

class MasterTrack : public Track {
public:
    MasterTrack(Composer* composer);
    void process(int64_t steadyTime) override;
    void renderLine(int line) override;
};

