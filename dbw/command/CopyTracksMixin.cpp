#include "CopyTracksMixin.h"
#include "../Track.h"

command::CopyTracksMixin::CopyTracksMixin(std::vector<Track*> tracks) : SelectedTracksMixin(tracks){
}

std::vector<Track*> command::CopyTracksMixin::copy(std::vector<NekoId> trackIds) {
    std::vector<Track*> tracks;
    for (auto& trackId : trackIds) {
        Track* track = Neko::findByNekoId<Track>(trackId);
        if (!track) {
            continue;
        }
        nlohmann::json json = track->toJson();
        json = renewNekoId(json);
        Track* copiedTrack = new Track(json);
        copiedTrack->resolveModuleReference();
        _copiedTrackIds.push_back(copiedTrack->getNekoId());
        tracks.push_back(copiedTrack);
    }
    return tracks;
}
