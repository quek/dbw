#include "PasteTracks.h"
#include "../App.h"
#include "../Composer.h"
#include "../GainModule.h"
#include "../Fader.h"
#include "../Track.h"

command::PasteTracks::PasteTracks(const nlohmann::json& jsonTracks, Track* at) :
    _jsonTracks(jsonTracks), _atTrackId(at->nekoId()) {
}

void command::PasteTracks::execute(Composer* composer) {
    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
    std::vector<Track*> tracks;
    for (const auto& x : _jsonTracks) {
        tracks.push_back(new Track(x));
    }

    for (auto& track : tracks) {
        track->resolveModuleReference();
    }

    std::vector<Track*> allTracks;
    for (auto& track : tracks) {
        track->allTracks(allTracks);
    }

    for (auto& track : allTracks) {
        for (auto& module : track->allModules()) {
            std::erase_if(module->_connections, [&allTracks](auto& c) {
                return std::ranges::all_of(allTracks, [&c](auto& track) { return c->_from->_track != track; }) ||
                    std::ranges::all_of(allTracks, [&c](auto& track) { return c->_to->_track != track; });
            });
        }
    }
     
    Track* atTrack = Neko::findByNekoId<Track>(_atTrackId);
    if (atTrack) {
        if (atTrack->isMasterTrack()) {
            for (const auto& track : tracks | std::views::reverse) {
                std::unique_ptr<Track> p(track);
                atTrack->insertTrack(atTrack->tracksBegin(), p);
            }
        } else {
            Track* parent = atTrack->getParent();
            for (const auto& track : tracks | std::views::reverse) {
                auto at = parent->findTrack(atTrack);
                std::unique_ptr<Track> p(track);
                parent->insertTrack(at + 1, p);
            }
        }
    }

    composer->computeProcessOrder();
}

void command::PasteTracks::undo(Composer* composer) {
    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
    Track* atTrack = Neko::findByNekoId<Track>(_atTrackId);
    auto parent = atTrack->getParent();
    auto it = parent->findTrack(atTrack);
    std::vector<Track*> tracks;
    for (int i = 0; i < _jsonTracks.size(); ++i) {
        tracks.emplace_back((*(it + i)).get());
    }
    composer->_masterTrack->deleteTracks(tracks);

    composer->computeProcessOrder();
}
