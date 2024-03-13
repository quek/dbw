#include "Wav.h"
#define DR_WAV_IMPLEMENTATION
#include <dr_libs/dr_wav.h>
#include "Config.h"
#include "ProcessBuffer.h"

Wav::Wav(const std::filesystem::path& file)
{
    _data = drwav_open_file_and_read_pcm_frames_f32(file.string().c_str(), &_nchannels, &_sampleRate, &_totalPCMFrameCount, nullptr);
    if (_data == nullptr)
    {
        // Error opening and reading WAV file.
    }
}

Wav::~Wav()
{
    if (_data)
    {
        drwav_free(_data, nullptr);
        _data = nullptr;
    }
}

uint32_t Wav::copy(ProcessBuffer& processBuffer, int frameOffset, double start, double end, double oneBeatSec)
{
    processBuffer.ensure32();
    double sampleRate = gPreference.sampleRate;
    auto nchannels = std::min(processBuffer._nchannels, _nchannels);
    double startFrameDouble = start * oneBeatSec * sampleRate;
    uint64_t startFrame = std::round(startFrameDouble);
    if (_totalPCMFrameCount < startFrame)
    {
        return;
    }
    double endFrameDouble = end * oneBeatSec * sampleRate;
    uint64_t endFrame = std::round(endFrameDouble);
    uint64_t nframes = std::min(std::min(endFrame - startFrame, _totalPCMFrameCount),
                                static_cast<uint64_t>(processBuffer._framesPerBuffer - frameOffset));
    std::vector<std::vector<float>>& buffer = processBuffer._out[0].buffer32();
    for (unsigned int channel = 0; channel < nchannels; ++channel)
    {
        for (uint64_t i = 0; i < nframes; ++i)
        {
            buffer[channel][i + frameOffset] = _data[_nchannels * (startFrame + i) + channel];
        }
        processBuffer._out[0]._constantp[channel] = false;
    }
    return nframes;
}

double Wav::getDuration(double bpm)
{
    double sampleRate = gPreference.sampleRate;
    double oneBeatSec = 60.0 / bpm;
    double duration = _totalPCMFrameCount / sampleRate / oneBeatSec;
    return duration;
}
