#pragma once
#include <map>
#include "GridMixin.h"
#include "ZoomMixin.h"

class Composer;
struct ImVec2;
class Track;

class TimelineWindow : public GridMixin, public ZoomMixin {
public:
    TimelineWindow(Composer* composer);
    void render();

private:
    void handleCanvas(ImVec2& clipRectMin, ImVec2& clipRectMax);
    void renderTimeline();
    void renderTrackHeader();
    void renderClips(ImVec2& windowPos);
    float getTrackWidth(Track* track);
    float allTracksWidth();

    Composer* _composer;
    std::map<Track*, float> _trackWidthMap;
};
