#include "Wav.h"
#define DR_WAV_IMPLEMENTATION
#include <cppcodec/base64_rfc4648.hpp>
#include <dr_libs/dr_wav.h>
#include "Config.h"
#include "ProcessBuffer.h"

Wav::Wav(const nlohmann::json& json)
{
    _nchannels = json["_nchannels"];
    _sampleRate = json["_sampleRate"];
    _totalPCMFrameCount = json["_totalPCMFrameCount"];
    size_t size = sizeof(float) * _totalPCMFrameCount * _nchannels;
    _data = (float*)malloc(size);
    std::string data = json["_data"];
    std::vector<uint8_t> buffer = cppcodec::base64_rfc4648::decode(data);
    memcpy(_data, buffer.data(), size);
}

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
    int64_t startFrame = std::round(startFrameDouble);
    if (static_cast<int64_t>(_totalPCMFrameCount) < startFrame)
    {
        return 0;
    }
    double endFrameDouble = end * oneBeatSec * sampleRate;
    int64_t endFrame = std::round(endFrameDouble);
    int64_t nframes = std::min(std::min(endFrame - startFrame, static_cast<int64_t>(_totalPCMFrameCount) - startFrame),
                               static_cast<int64_t>(processBuffer._framesPerBuffer - frameOffset));
    std::vector<std::vector<float>>& buffer = processBuffer._out[0].buffer32();
    for (unsigned int channel = 0; channel < nchannels; ++channel)
    {
        for (int64_t i = 0; i < nframes; ++i)
        {
            if (startFrame + i >= 0)
            {
                buffer[channel][i + frameOffset] = _data[_nchannels * (startFrame + i) + channel];
            }
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

nlohmann::json Wav::toJson()
{
    nlohmann::json json;
    json["_data"] = cppcodec::base64_rfc4648::encode((const char*)_data, _totalPCMFrameCount * _nchannels);
    json["_nchannels"] = _nchannels;
    json["_sampleRate"] = _sampleRate;
    json["_totalPCMFrameCount"] = _totalPCMFrameCount;
    return json;
}
