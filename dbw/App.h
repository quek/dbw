#pragma once
#include <memory>
#include <mutex>
#include <vector>
#include "AudioEngine.h"
#include "AudioEngineWindow.h"
#include "Composer.h"

class App {
public:
    App();
    void render();
    AudioEngine* audioEngine() const { return _audioEngine.get(); }
    std::vector<std::unique_ptr<Composer>>& composers() { return _composers; }
    void addComposer(Composer* composer);
    void deleteComposer(Composer* composer);
    void runCommand();
    void requestAddComposer(Composer* composer);
    void requestDeleteComposer(Composer* composer);
    void showAudioSetupWindow();
    bool isStarted() const { return _isStarted; }
    void start();
    void stop();

    std::recursive_mutex _mtx;

private:
    bool _isStarted = false;
    std::unique_ptr<AudioEngine> _audioEngine;
    std::unique_ptr<AudioEngineWindow> _audioEngineWindow = nullptr;
    std::vector<std::unique_ptr<Composer>> _composers;

    std::vector<Composer*> _requestAddComposers;
    std::vector<Composer*> _requestDeleteComposers;

};

