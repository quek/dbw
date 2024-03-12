#pragma once
#include <filesystem>
#include <dr_libs/dr_wav.h>

class Wav {
public:
    Wav(const std::filesystem::path& file);
    virtual ~Wav();
private:
    float* _data = nullptr;
    unsigned int _nchannels = 0;
    unsigned int _sampleRate = 0;
    drwav_uint64 _totalPCMFrameCount = 0;
};

