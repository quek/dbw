#pragma once
#include <memory>
#include <string>
#include "AudioEngineWindow.h"
#include "SaveWindow.h"

class Composer;

class ComposerWindow {
public:
    ComposerWindow(Composer* composer);
    void render();
    void setStatusMessage(std::string message);

    Composer* _composer;
    bool _showAudioEngineWindow = false;
    bool _showSaveWindow = false;
private:
    void handleGlobalShortcut();

    std::unique_ptr<AudioEngineWindow> _audioEngineWindow = nullptr;
    std::unique_ptr<SaveWindow> _saveWindow = nullptr;
    std::string _statusMessage = "";
};
