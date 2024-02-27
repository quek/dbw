#pragma once
#include "../Command.h"

namespace command {
class DeleteTracks : public Command {
public:
    DeleteTracks(std::vector<Track*>& tracks);
    virtual ~DeleteTracks() = default;
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;

protected:
    std::vector<NekoId> _trackIds;
    nlohmann::json _jsonTracks;
    std::vector<std::pair<NekoId, std::ptrdiff_t>> _undoPlaces;
};
};

