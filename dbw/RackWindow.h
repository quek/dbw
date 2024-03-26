#pragma once
#include <map>
#include <vector>
#include "BaseWindow.h"

class Composer;
class Module;
class Track;

class RackWindow : public BaseWindow {
public:
    RackWindow(Composer* composer);
    void render();

private:
    void renderHeader();
    void renderHeader(Track* track, int groupLevel, bool isMaster, bool adjustY);
    void renderModules();
    void renderModules(Track* track);
    void renderFaders();
    void renderFaders(Track* track);
    void handleShortcut();
    void computeHeaderHeight();
    void computeHeaderHeight(Track* track,int groupLevel);

    Composer* _composer;
    std::vector<Track*> _allTracks;
    std::vector<Module*> _selectedModules;
    std::vector<Track*>& _selectedTracks;
    float _headerHeight = 0.0f;
    float _faderHeight = 200.0f;
    ImVec2 _groupToggleButtonSize = ImVec2(16.0f, 22.0f);
    Track* _renamingTrack = nullptr;
};

