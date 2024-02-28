#pragma once
#include <vector>
#include "../Command.h"
#include "RemoveTracksMixin.h"

namespace command {
class GroupTracks : public Command,  public RemoveTracksMixin {
public:
    GroupTracks(std::vector<Track*> tracks, bool undoable);
    virtual ~GroupTracks() = default;
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;
private:
    NekoId _groupId = 0;
};
};
