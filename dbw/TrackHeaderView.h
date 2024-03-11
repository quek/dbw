#pragma once

class Composer;
struct ImVec2;
class Lane;
class Track;
class TrackWidthManager;

class TrackHeaderView {
public:
    TrackHeaderView(Composer* composer, TrackWidthManager& trackWidthManager);
    float render(float offsetX, float zoomX);

private:
    void computeHeaderHeight();
    void computeHeaderHeight(Track* track, int groupLevel);
    float getTrackWidth(Track* track);
    float getLaneWidth(Lane* lane);
    ImVec2 posScreenToWindow(const ImVec2& pos);
    ImVec2 posWindowToScreen(const ImVec2& pos);
    void renderLane(Lane* lane, int groupLevel);
    void renderTrack(Track* track, int groupLevel);

    Composer* _composer;
    TrackWidthManager& _trackWidthManager;
    float _headerHeight = 0.0f;
    float _scrollX = 0.0f;
    float _scrollY = 0.0f;
    float _x = 0.0f;
    float _zoomX = 1.0f;
};

