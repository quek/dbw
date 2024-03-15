#include "DeleteTracks.h"
#include "../App.h"
#include "../Composer.h"
#include "../SerializeContext.h"
#include "../Track.h"

command::DeleteTracks::DeleteTracks(std::vector<Track*>& tracks) : RemoveTracksMixin(tracks) {
}

void command::DeleteTracks::execute(Composer* composer) {
    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);

    SerializeContext context;
    nlohmann::json array = nlohmann::json::array();
    std::vector<std::unique_ptr<Track>> removedTracks = removeTracks();
    for (auto& track : removedTracks) {
            array.push_back(track->toJson(context));
    }
    _jsonTracks["tracks"] = array;

    composer->computeProcessOrder();
}

void command::DeleteTracks::undo(Composer* composer) {
    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);

    std::vector<std::unique_ptr<Track>> tracks;
    for (auto& jsonTrack : _jsonTracks["tracks"]) {
        tracks.emplace_back(new Track(jsonTrack));
    }
    undoRemove(composer, tracks);
}

