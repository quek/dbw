#include "MasterTrack.h"

MasterTrack::MasterTrack(const nlohmann::json& json, SerializeContext& context) : Track(json, context) {
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

nlohmann::json MasterTrack::toJson(SerializeContext& context) {
    nlohmann::json json = Track::toJson(context);
    return json;
}
