#pragma once

#include <clap/clap.h>
#include <portaudio.h>

class App;

class AudioEngine
{
public:
    AudioEngine(App* app);
    ~AudioEngine();
    void start();
    void stop();
    void process(float* in, float* out, unsigned long framesPerBuffer);

    App* _app = nullptr;
    bool _isStarted = false;
    double _cpuLoad = 0.0;

private:
    PaStream* _stream = nullptr;
    int64_t _steadyTime = 0;
};

