#include "Audio.h"
#include "implot/implot.h"
#include "Lane.h"
#include "ProcessBuffer.h"
#include "Track.h"
#include "util.h"

Audio::Audio(const nlohmann::json& json, SerializeContext& context) : SequenceItem(json, context)
{
    _audioFile = std::make_unique<AudioFile>(json["_audioFile"], context);
}

Audio::Audio(const std::filesystem::path& path, double bpm) : _audioFile(new AudioFile(path))
{
    _duration = _audioFile->getDuration(bpm);
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
        frameOffset = _audioFile->copy(processBuffer, frameOffset, wavBegin, wavEnd, oneBeatSec);
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

    //int count = _audioFile->getNframes();
    //uint32_t nchannels = _audioFile->getNchannels();
    //const float* data = _audioFile->getData();
    //std::unique_ptr<float[]> xs(new float[count]);
    //for (int i = 0; i < count; ++i)
    //{
    //    xs[i] = data[nchannels * i];
    //}
    //ImGui::PlotLines("_", xs.get(), count, 0, nullptr, -1.0f, 1.0f, size);

    ImPlotFlags plotFlags = ImPlotFlags_NoTitle | ImPlotFlags_NoLegend | ImPlotFlags_NoMouseText | ImPlotFlags_NoInputs | ImPlotFlags_NoMenus | ImPlotFlags_NoBoxSelect | ImPlotFlags_NoFrame;
    if (ImPlot::BeginPlot("Audio", size, plotFlags))
    {
        int count = _audioFile->getNframes();
        uint32_t nchannels = _audioFile->getNchannels();
        float* data = _audioFile->getData();
        std::unique_ptr<float[]> xs(new float[count]);
        std::unique_ptr<float[]> ys(new float[count]);
        for (int i = 0; i < count; ++i)
        {
            xs[i] = data[nchannels * i];
            ys[i] = count - i;
        }

        ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(0, 0));
        ImPlotAxisFlags axisFlags = ImPlotAxisFlags_NoLabel | ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoMenus | ImPlotAxisFlags_NoSideSwitch;
        ImPlot::SetupAxes("x", "y", axisFlags, axisFlags);
        ImPlot::PlotLine("l", xs.get(), ys.get(), count);
        ImPlot::PopStyleVar();
        ImPlot::EndPlot();
    }
}

nlohmann::json Audio::toJson(SerializeContext& context)
{
    nlohmann::json json = SequenceItem::toJson(context);
    json["type"] = TYPE;
    json["_audioFile"] = _audioFile->toJson(context);

    return json;
}
