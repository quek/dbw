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
    std::vector<uint64_t> _ids;
    std::vector<std::pair<uint64_t, std::ptrdiff_t>> _undoPlaces;
};
};
