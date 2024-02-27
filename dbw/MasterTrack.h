#pragma once
#include "Track.h"

class MasterTrack : public Track {
public:
    MasterTrack(const nlohmann::json& json);
    MasterTrack(Composer* composer);
    bool isMasterTrack() override { return true; }
    nlohmann::json toJson() override;
};

