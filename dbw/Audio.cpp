#include "Audio.h"
#include "Lane.h"
#include "ProcessBuffer.h"
#include "Track.h"

Audio::Audio(const nlohmann::json& json) : SequenceItem(json)
{
    _wav = std::make_unique<Wav>(json["_wav"]);
    std::wstring wavPath = json["_wavPath"];
    _wavPath = wavPath;
}

Audio::Audio(const std::filesystem::path& wavPath, double bpm) : _wavPath(wavPath), _wav(new Wav(wavPath))
{
    _duration = _wav->getDuration(bpm);
}

void Audio::prepareProcessBuffer(Lane* lane, double begin, double end, double clipBegin, double clipEnd, double loopBegin, double loopEnd, double oneBeatSec)
{
    ProcessBuffer& processBuffer = lane->_track->_processBuffer;
    uint32_t frameOffset = 0;
    double wavBegin = begin - clipBegin;
    if (begin < end || loopEnd <= begin)
    {
        double wavEnd = wavBegin + (end - begin);
        frameOffset = _wav->copy(processBuffer, frameOffset, wavBegin, wavEnd, oneBeatSec);
    }
    else
    {
        double duration = loopEnd - begin;
        double wavEnd = wavBegin + duration;
        frameOffset = _wav->copy(processBuffer, frameOffset, wavBegin, wavEnd, oneBeatSec);
        wavBegin = loopBegin - clipBegin;
        wavEnd = end - clipBegin;
        frameOffset = _wav->copy(processBuffer, frameOffset, wavBegin, wavEnd, oneBeatSec);
    }
    if (frameOffset == 0)
    {
        for (unsigned int channel = 0; channel < processBuffer._nchannels; ++channel)
        {
            processBuffer._out[0].buffer32()[channel][0] = 0.0f;
            processBuffer._out[0]._constantp[channel] = true;
        }
    }
    else
    {
        for (uint32_t i = frameOffset; i < processBuffer._framesPerBuffer; ++i)
        {
            for (unsigned int channel = 0; channel < processBuffer._nchannels; ++channel)
            {
                processBuffer._out[0].buffer32()[channel][i] = 0.0f;
            }
        }
    }
}

nlohmann::json Audio::toJson()
{
    nlohmann::json json = SequenceItem::toJson();
    json["type"] = TYPE;
    json["_wav"] = _wav->toJson();
    json["_wavPath"] = _wavPath.wstring();

    return json;
}
