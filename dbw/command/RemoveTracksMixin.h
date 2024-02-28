#pragma once
#include <memory>
#include <vector>
#include <nlohmann/json.hpp>
#include "../Neko.h"

class Composer;
class Track;

namespace command {
class RemoveTracksMixin {
public:
    RemoveTracksMixin(std::vector<Track*>& tracks);
    virtual ~RemoveTracksMixin() = default;

protected:
    bool isChild(Track* track, const std::vector<Track*> tracks);
    std::vector<Track*> removeChildren(const std::vector<Track*> tracks);
    std::vector<std::unique_ptr<Track>> removeTracks();
    void undoRemove(Composer* composer, std::vector<std::unique_ptr<Track>>& tracks);

    std::vector<NekoId> _trackIds;
    nlohmann::json _jsonTracks;
    std::vector<std::pair<NekoId, std::ptrdiff_t>> _undoPlaces;
};
};

