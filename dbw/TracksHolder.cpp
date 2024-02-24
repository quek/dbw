#include "TracksHolder.h"
#include "Track.h"

TracksHolder::TracksHolder(const nlohmann::json& json) :
    Nameable(json) {
    // TODO _tracks まわりをここに共通化
}

TracksHolder::TracksHolder(const std::string& name) :
    Nameable(name) {
}

void TracksHolder::addTrack() {
    auto name = std::to_string(_tracks.size() + 1);
    Track* track = new Track(name);
    addTrack(std::unique_ptr<Track>(track));
}

void TracksHolder::addTrack(Track* track) {
    addTrack(std::unique_ptr<Track>(track));
}

void TracksHolder::addTrack(std::unique_ptr<Track> track) {
    track->setTracksHolder(this);
    _tracks.emplace_back(std::move(track));
}

std::vector<std::unique_ptr<Track>>::iterator TracksHolder::findTrack(Track* track) {
    return std::ranges::find_if(_tracks, [track](const auto& x) { return x.get() == track; });
}

std::vector<std::unique_ptr<Track>>& TracksHolder::getTracks() {
    return _tracks;
}

