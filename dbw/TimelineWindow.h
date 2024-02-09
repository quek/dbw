#pragma once
#include <map>
#include <set>
#include "GridMixin.h"
#include "ZoomMixin.h"

class Clip;
class Composer;
struct ImVec2;
class Track;

class TimelineWindow : public GridMixin, public ZoomMixin {
public:
    TimelineWindow(Composer* composer);
    void render();

private:
    void handleCanvas(ImVec2& clipRectMin, ImVec2& clipRectMax);
    void handleShortcut();
    void renderTimeline();
    void renderTrackHeader();
    void renderClips(ImVec2& windowPos);
    float getTrackWidth(Track* track);
    float allTracksWidth();

    Composer* _composer;
    std::map<Track*, float> _trackWidthMap;

    enum ClipClickedPart {
        Top,
        Middle,
        Bottom
    };
    struct State {
        Clip* _clickedClip = nullptr;
        std::set<Clip*> _selectedClips;
        Clip* _draggingClip = nullptr;
        bool _unselectClickedClipIfMouserReleased = false;
        ClipClickedPart _clipClickedPart = Middle;
        float _clipClickedOffset = 0.0f;
        bool _rangeSelecting = false;

        bool _consumedDoubleClick = false;
        bool _consumedClicked = false;
        void reset();
    };
    State _state;
};
