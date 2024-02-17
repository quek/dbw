#pragma once
#include "Track.h"

class Composer;

class MasterTrack : public Track {
public:
    MasterTrack(Composer* composer);
    void process(int64_t steadyTime) override;

private:
    virtual const char* role() { return "master"; }
};
