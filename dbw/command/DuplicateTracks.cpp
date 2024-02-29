#include "DuplicateTracks.h"
#include "../App.h"
#include "../Composer.h"
#include "../MasterTrack.h"
#include "../Track.h"

command::DuplicateTracks::DuplicateTracks(std::vector<Track*> tracks) :
    CopyTracksMixin(tracks) {
    _atTrackId = _selectedTrackIds.back();

}

void command::DuplicateTracks::insertTracks(std::vector<std::unique_ptr<Track>>& tracks, Track* at) {
    at->insertTracksAfterThis(tracks);
}

