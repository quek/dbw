#include "MasterTrack.h"

MasterTrack::MasterTrack(const nlohmann::json& json) : Track(json) {
    _parent = this;
}

MasterTrack::MasterTrack(Composer* composer) : Track("MASTER", composer) {
}

std::vector<std::unique_ptr<Track>>::iterator MasterTrack::getAt() {
    return _tracks.begin();
}

MasterTrack* MasterTrack::getMasterTrack() {
    return this;
}

nlohmann::json MasterTrack::toJson() {
    nlohmann::json json = Track::toJson();
    return json;
}
