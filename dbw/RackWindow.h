#pragma once
#include <map>
#include <vector>
#include "BaseWindow.h"

class Composer;
class Track;

class RackWindow : public BaseWindow {
public:
    RackWindow(Composer* composer);
    void render();

private:
    void renderHeader();
    void renderHeader(Track* track, int groupLevel, bool isMaster);
    void renderModules();
    void renderFaders();
    void handleShortcut();
    void computeHeaderHeight();
    void computeHeaderHeight(Track* track,int groupLevel);

    Composer* _composer;
    std::vector<Track*> _allTracks;
    std::vector<Track*> _selectedTracks;
    float _headerHeight = 0.0f;
    float _faderHeight = 150.0f;
    ImVec2 _groupToggleButtonSize = ImVec2(16.0f, 22.0f);
};

