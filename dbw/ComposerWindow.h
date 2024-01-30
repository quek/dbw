#pragma once
#include <string>

class Composer;

class ComposerWindow {
public:
    ComposerWindow(Composer* composer);
    void render();
    void setStatusMessage(std::string message);
private:
    Composer* _composer;
    std::string _statusMessage = "";
    float _lastTracksScrollX = 0.0f;
    bool _tracksScrolled = false;
    float _lastRacksScrollX = 0.0f;
    bool _racksScrolled = false;
};
