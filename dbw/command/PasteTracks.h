#pragma once
#include "../Command.h"

class Track;

namespace command {

class PasteTracks : public Command {
public:
    PasteTracks(const nlohmann::json& tracks, Track* at);
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;

private:
    nlohmann::json _jsonTracks;
    NekoId _atTrackId;
};

};

