#pragma once
#include <memory>
#include <vector>
#include <nlohmann/json.hpp>
#include "SelectedTracksMixin.h"

class Composer;
class Track;

namespace command {
class RemoveTracksMixin : public SelectedTracksMixin {
public:
    RemoveTracksMixin(std::vector<Track*>& tracks);
    virtual ~RemoveTracksMixin() = default;

protected:
    std::vector<std::unique_ptr<Track>> removeTracks();
    void undoRemove(Composer* composer, std::vector<std::unique_ptr<Track>>& tracks);

    nlohmann::json _jsonTracks;
    std::vector<std::pair<NekoId, std::ptrdiff_t>> _undoPlaces;
};
};

