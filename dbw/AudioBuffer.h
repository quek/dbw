#pragma once
#include <vector>

class AudioBuffer {
public:
    AudioBuffer();
    void add(const AudioBuffer& other);
    void copyFrom(const AudioBuffer& other);
    void copyFrom(const float** buffer, unsigned long framesPerBuffer, int nchannels);
    void copyTo(float** buffer, unsigned long framesPerBuffer, int nchannels);
    void copyTo(float* buffer, unsigned long framesPerBuffer, int nchannels);
    void ensure(unsigned long framesPerBuffer, int nchannels);
    int getNchannels() const { return _nchannels; }
    void zero();

    std::vector<std::vector<float>> _buffer;
    std::vector<bool> _constantp;

private:
    unsigned long _framesPerBuffer;
    int _nchannels = 2;
};

