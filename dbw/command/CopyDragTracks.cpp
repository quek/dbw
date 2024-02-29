#include "CopyDragTracks.h"
#include "../Track.h"

command::CopyDragTracks::CopyDragTracks(std::vector<Track*> tracks, Track* at) :
    CopyTracksMixin(tracks) {
    _atTrackId = at->getNekoId();
}

void command::CopyDragTracks::insertTracks(std::vector<std::unique_ptr<Track>>& tracks, Track* at) {
    at->insertTracksBeforeThis(tracks);
}
