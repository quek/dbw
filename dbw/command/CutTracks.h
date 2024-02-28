#pragma once
#include "DeleteTracks.h"

namespace command {
class CutTracks : public DeleteTracks {
public:
    CutTracks(std::vector<Track*>& tracks);
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;
};
};
