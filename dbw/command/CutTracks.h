#pragma once
#include "../Command.h"

namespace command {
class CutTracks : public Command {
public:
    CutTracks(std::vector<Track*>& tracks);
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;

private:
    std::vector<NekoId> _trackIds;
    nlohmann::json _jsonTracks;
    std::vector<std::pair<NekoId, std::ptrdiff_t>> _undoPlaces;
};
};
