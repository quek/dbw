#pragma once

#include <mutex>
#include <clap/clap.h>
#include <portaudio.h>

class Composer;

class AudioEngine
{
public:
    AudioEngine();
    ~AudioEngine();
    void start();
    void stop();
    void process(float* in, float* out, unsigned long framesPerBuffer);

    Composer* _composer = nullptr;
    std::mutex mtx;
    bool _isStarted = false;
private:
    PaStream* _stream = nullptr;
    int64_t _steadyTime = 0;
};

