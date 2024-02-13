#pragma once
#include <vector>

class AudioBuffer {
public:
    AudioBuffer();
    void add(const AudioBuffer& other);
    void copyFrom(const float** buffer, unsigned long framesPerBuffer, int nchannels);
    void copyFrom(const double** buffer, unsigned long framesPerBuffer, int nchannels);
    void copyTo(float** buffer, unsigned long framesPerBuffer, int nchannels);
    void copyTo(float* buffer, unsigned long framesPerBuffer, int nchannels);
    void ensure(unsigned long framesPerBuffer, int nchannels);
    void ensure32();
    void ensure64();
    int getNchannels() const { return _nchannels; }
    void zero();
    std::vector<std::vector<float>>& buffer32();
    std::vector<std::vector<double>>& buffer64();

    std::vector<bool> _constantp;

    enum DataType {
        Float,
        Double,
    };
    DataType _dataType = Float;
private:
    unsigned long _framesPerBuffer;
    int _nchannels = 2;
    std::vector<std::vector<float>> _buffer32;
    std::vector<std::vector<double>> _buffer64;
};
