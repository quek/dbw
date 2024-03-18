#include "Audio.h"
#include "Lane.h"
#include "PlotLinesV.h"
#include "ProcessBuffer.h"
#include "Track.h"
#include "util.h"

Audio::Audio(const nlohmann::json& json, SerializeContext& context) : SequenceItem(json, context)
{
    _audioFile = std::make_unique<AudioFile>(json["_audioFile"], context);
}

Audio::Audio(const std::filesystem::path& path, double bpm) : _audioFile(new AudioFile(path))
{
    _duration = _audioFile->durationGet(bpm);
}

void Audio::prepareProcessBuffer(Lane* lane, double begin, double end, double clipBegin, double /*clipEnd*/, double loopBegin, double loopEnd, double oneBeatSec)
{
    ProcessBuffer& processBuffer = lane->_track->_processBuffer;
    uint32_t frameOffset = 0;
    double wavBegin = begin - clipBegin;
    if (begin < end || loopEnd <= begin)
    {
        double wavEnd = wavBegin + (end - begin);
        frameOffset = _audioFile->copy(processBuffer, frameOffset, wavBegin, wavEnd, oneBeatSec);
    }
    else
    {
        double duration = loopEnd - begin;
        double wavEnd = wavBegin + duration;
        frameOffset = _audioFile->copy(processBuffer, frameOffset, wavBegin, wavEnd, oneBeatSec);
        wavBegin = loopBegin - clipBegin;
        wavEnd = end - clipBegin;
        frameOffset += _audioFile->copy(processBuffer, frameOffset, wavBegin, wavEnd, oneBeatSec);
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

void Audio::render(const ImVec2& pos1, const ImVec2& pos2, const bool selected)
{
    ImGui::SetCursorPos(pos1 - ImGui::GetWindowPos() + ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY()));
    ImVec2 size = pos2 - pos1;
    int count = _audioFile->getNframes();

    if (_plotPos1 != pos1 || _plotPos2 != pos2)
    {
        _plotPos1 = pos1;
        _plotPos2 = pos2;
        uint32_t nchannels = _audioFile->getNchannels();
        float* data = _audioFile->getData();
        _plotData.reset(new float[count]);
        for (int i = 0; i < count; ++i)
        {
            _plotData[i] = data[nchannels * i];
        }
    }
    ImGui::PushID(this);
    ImGui::PlotLinesV("##_", _plotData.get(), count, 0, -1.0f, 1.0f, size);
    ImGui::PopID();
}

nlohmann::json Audio::toJson(SerializeContext& context)
{
    nlohmann::json json = SequenceItem::toJson(context);
    json["type"] = TYPE;
    json["_audioFile"] = _audioFile->toJson(context);

    return json;
}
