#pragma once
#include <memory>
#include <mutex>
#include <vector>
#include "AudioEngine.h"
#include "Composer.h"

class App {
public:
    App();
    AudioEngine* audioEngine() const { return _audioEngine.get(); }
    std::vector<std::unique_ptr<Composer>>& composers() { return _composers; }
    void addComposer(Composer* composer);
    void deleteComposer(Composer* composer);
    void runCommand();
    void requestAddComposer(Composer* composer);
    void requestDeleteComposer(Composer* composer);

    std::recursive_mutex _mtx;

private:
    std::unique_ptr<AudioEngine> _audioEngine;
    std::vector<std::unique_ptr<Composer>> _composers;
    std::vector<Composer*> _requestAddComposers;
    std::vector<Composer*> _requestDeleteComposers;
};

