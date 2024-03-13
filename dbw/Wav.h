#pragma once
#include <filesystem>
#include <dr_libs/dr_wav.h>

class ProcessBuffer;

class Wav
{
public:
    Wav(const std::filesystem::path& file);
    virtual ~Wav();
    uint32_t copy(ProcessBuffer& processBuffer, int frameOffset, double start, double end, double oneBeatSec);
    uint64_t getNframes() { return _totalPCMFrameCount; }
    double getDuration(double bpm);
private:
    float* _data = nullptr;
    unsigned int _nchannels = 0;
    unsigned int _sampleRate = 0;
    drwav_uint64 _totalPCMFrameCount = 0;
};

