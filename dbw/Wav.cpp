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

void Wav::copy(ProcessBuffer& processBuffer, double start, double /*end*/, double oneBeatSec)
{
    processBuffer.ensure32();
    double sampleRate = gPreference.sampleRate;
    auto nchannels = std::min(processBuffer._nchannels, _nchannels);
    double startFrameDouble = start * oneBeatSec * sampleRate;
    uint32_t startFrame = std::round(startFrameDouble);
    std::vector<std::vector<float>>& buffer = processBuffer._in[0].buffer32();
    for (uint32_t i = 0; i < processBuffer._framesPerBuffer && startFrame + i < _totalPCMFrameCount; ++i)
    {
        for (unsigned int channel = 0; channel < nchannels; ++channel)
        {
            buffer[channel][i] = _data[_nchannels * (startFrame + i) + channel];

        }
    }

}
