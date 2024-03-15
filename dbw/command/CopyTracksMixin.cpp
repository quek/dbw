#include "CopyTracksMixin.h"
#include "../App.h"
#include "../Composer.h"
#include "../SerializeContext.h"
#include "../Track.h"

command::CopyTracksMixin::CopyTracksMixin(std::vector<Track*> tracks) : SelectedTracksMixin(tracks){
}

void command::CopyTracksMixin::execute(Composer* composer) {
    std::vector<std::unique_ptr<Track>> tracks;
    for (auto& track : copy(_selectedTrackIds)) {
        tracks.emplace_back(track);
    }
    if (tracks.empty()) {
        return;
    }
    Track* at = Neko::findByNekoId<Track>(_atTrackId);
    if (!at) {
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
    insertTracks(tracks, at);
    composer->computeProcessOrder();
}

void command::CopyTracksMixin::undo(Composer* composer) {
    std::vector<Track*> tracks;
    for (auto& cpiedTrackId : _copiedTrackIds) {
        Track* track = Neko::findByNekoId<Track>(cpiedTrackId);
        if (!track) {
            continue;
        }
        tracks.push_back(track);
    }
    if (tracks.empty()) {
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
    tracks[0]->getMasterTrack()->deleteTracks(tracks);
    composer->computeProcessOrder();
}

std::vector<Track*> command::CopyTracksMixin::copy(std::vector<NekoId> trackIds) {
    std::vector<Track*> tracks;
    for (auto& trackId : trackIds) {
        Track* track = Neko::findByNekoId<Track>(trackId);
        if (!track) {
            continue;
        }
        SerializeContext context;
        nlohmann::json json = track->toJson(context);
        json = renewNekoId(json);
        Track* copiedTrack = new Track(json, context);
        copiedTrack->resolveModuleReference();
        _copiedTrackIds.push_back(copiedTrack->getNekoId());
        tracks.push_back(copiedTrack);
    }
    return tracks;
}
