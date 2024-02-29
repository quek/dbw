#include "CopyDragTracks.h"
#include "../Track.h"

command::CopyDragTracks::CopyDragTracks(std::vector<Track*> tracks, Track* at) :
    CopyTracksMixin(tracks), _atTrackId(at->getNekoId())  {
}

void command::CopyDragTracks::execute(Composer* composer) {
}

void command::CopyDragTracks::undo(Composer* composer) {
}
