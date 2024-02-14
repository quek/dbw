#pragma once
#include <memory>
#include <string>
#include "SaveWindow.h"

class Composer;

class ComposerWindow {
public:
    ComposerWindow(Composer* composer);
    void render();
    void setStatusMessage(std::string message);

    Composer* _composer;
    bool _showSaveWindow = false;
private:
    std::unique_ptr<SaveWindow> _saveWindow = nullptr;
    std::string _statusMessage = "";
};
