#include "MasterTrack.h"

MasterTrack::MasterTrack(const nlohmann::json& json) : Track(json) {
}

MasterTrack::MasterTrack(Composer* composer) : Track("MASTER", composer) {
}

nlohmann::json MasterTrack::toJson() {
    nlohmann::json json = Track::toJson();
    return json;
}
