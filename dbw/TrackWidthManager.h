#pragma once
#include <map>

class Lane;
class MasterTrack;
class Track;

class TrackWidthManager {
public:
    TrackWidthManager(MasterTrack* masterTrack);
    float allTracksWidth();
    float getLaneWidth(Lane* lane);
    float getTrackWidth(Track* track);

private:
    std::map<Lane*, float> _laneWidthMap;
    MasterTrack* _masterTrack;
};

