#include "TrackWidthManager.h"
#include "Composer.h"
#include "MasterTrack.h"
#include "Track.h"

TrackWidthManager::TrackWidthManager(MasterTrack* masterTrack) : _masterTrack(masterTrack) {
}

float TrackWidthManager::getLaneWidth(Lane* lane) {
    if (!_laneWidthMap.contains(lane)) {
        _laneWidthMap[lane] = 100.0f;
    }
    return _laneWidthMap[lane];
}

float TrackWidthManager::getTrackWidth(Track* track) {
    float width = 0.0f;
    for (auto& lane : track->_lanes) {
        width += getLaneWidth(lane.get());
    }
    return width;
}

float TrackWidthManager::allTracksWidth() {
    float width = getTrackWidth(_masterTrack);
    for (auto& track : _masterTrack->getTracks()) {
        width += getTrackWidth(track.get());
    }
    return width;
}
