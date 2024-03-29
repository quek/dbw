#pragma once
#include <memory>
#include <set>
#include <string>
#include "BaseWindow.h"
#include "Track.h"

class Composer;

class ComposerWindow : public BaseWindow{
public:
    ComposerWindow(Composer* composer);
    void render();
    void setStatusMessage(std::string message);

    Composer* _composer;
    bool _showAudioEngineWindow = false;
    bool _showSaveWindow = false;
private:
    void handleGlobalShortcut();
    void handleLocalShortcut();

    std::string _statusMessage = "";

    std::set<Track*> _selectedTracks;
};
