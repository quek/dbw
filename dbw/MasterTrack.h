#pragma once
#include "Track.h"

class MasterTrack : public Track {
public:
    MasterTrack(const nlohmann::json& json, SerializeContext& context);
    MasterTrack(Composer* composer);
    std::vector<std::unique_ptr<Track>>::iterator getAt() override;
    MasterTrack* getMasterTrack() override;
    bool isMasterTrack() override { return true; }
    nlohmann::json toJson(SerializeContext& context) override;
};

