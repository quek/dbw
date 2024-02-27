#pragma once
#include <vector>
#include "../Command.h"

namespace command {
class GroupTracks : public Command {
public:
    GroupTracks(std::vector<Track*> tracks, bool undoable);
    virtual ~GroupTracks() = default;
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;
private:
    NekoId _groupId = 0;
    std::vector<NekoId> _ids;
    std::vector<std::pair<NekoId, std::ptrdiff_t>> _undoPlaces;
};
};
