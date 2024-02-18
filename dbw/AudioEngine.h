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
    void restart(int deviceIndex, double sampleRate, unsigned long bufferSize);
    void process(float* in, float* out, unsigned long framesPerBuffer);

    double _sampleRate = 48000.0;
    unsigned long _bufferSize = 1024;

    Composer* _composer = nullptr;
    std::mutex mtx;
    bool _isStarted = false;
    int _deviceIndex = -1;
private:
    PaStream* _stream = nullptr;
    int64_t _steadyTime = 0;
};

