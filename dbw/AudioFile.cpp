#include "AudioFile.h"
#define DR_WAV_IMPLEMENTATION
#include <cppcodec/base64_rfc4648.hpp>
#include <libsndfile-1.2.2-win64/include/sndfile.hh>
#include "Config.h"
#include "ProcessBuffer.h"

AudioFile::AudioFile(const nlohmann::json& json, SerializeContext&)
{
    _nchannels = json["_nchannels"];
    _nframes = json["_nframes"];
    std::wstring path = json["_path"];
    _path = path;
    std::string data = json["_data"];
    _sampleRate = json["_sampleRate"];
    // sizeof(float) * _nframes * _nchannels だとなぜか 2 たりない場合があるから
    size_t size = cppcodec::base64_rfc4648::decoded_max_size(data.size());
    _data.reset(new float[size / sizeof(float) + sizeof(float)]);
    auto sz = cppcodec::base64_rfc4648::decode((char*)_data.get(), size, data.c_str(), data.size());
    assert(sizeof(float) * _nframes * _nchannels == sz);
}

AudioFile::AudioFile(const std::filesystem::path& path) : _path(path)
{
    SndfileHandle snd(path.wstring().c_str());
    if (!snd)
    {
        return;
    }
    size_t size = snd.frames() * snd.channels();
    _data.reset(new float[size]);
    snd.read(_data.get(), size);
    _nchannels = snd.channels();
    _sampleRate = snd.samplerate();
    _nframes = snd.frames();
}

uint32_t AudioFile::copy(ProcessBuffer& processBuffer, int frameOffset, double start, double end, double oneBeatSec)
{
    processBuffer.ensure32();
    double sampleRate = gPreference.sampleRate;
    auto nchannels = std::min(processBuffer._nchannels, _nchannels);
    double startFrameDouble = start * oneBeatSec * sampleRate;
    int64_t startFrame = std::round(startFrameDouble);
    double endFrameDouble = end * oneBeatSec * sampleRate;
    int64_t endFrame = std::round(endFrameDouble);
    int64_t nframes = std::min(std::min(endFrame - startFrame, static_cast<int64_t>(_nframes) - startFrame),
                               static_cast<int64_t>(processBuffer._framesPerBuffer - frameOffset));
    if (static_cast<int64_t>(_nframes) < startFrame)
    {
        return endFrame - startFrame;
    }
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

double AudioFile::getDuration(double bpm) const
{
    double sampleRate = gPreference.sampleRate;
    double oneBeatSec = 60.0 / bpm;
    double duration = _nframes / sampleRate / oneBeatSec;
    return duration;
}

nlohmann::json AudioFile::toJson(SerializeContext&)
{
    nlohmann::json json;
    std::string data = cppcodec::base64_rfc4648::encode((const char*)_data.get(), sizeof(float) * _nframes * _nchannels);
    json["_data"] = data;
    json["_nchannels"] = _nchannels;
    json["_nframes"] = _nframes;
    json["_path"] = _path.wstring();
    json["_sampleRate"] = _sampleRate;
    return json;
}
