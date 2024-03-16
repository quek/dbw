#include "Note.h"
#include "Config.h"
#include "Lane.h"
#include "ProcessBuffer.h"
#include "Track.h"

Note::Note(const nlohmann::json& json, SerializeContext& context) : SequenceItem(json, context)
{
    _channel = json["_channel"];
    _key = json["_key"];
    _velocity = json["_velocity"];
    _rel = json["_rel"];
}

Note::Note(double time, double duration, int16_t key, double velocity, int16_t channel) :
    SequenceItem(time, duration), _key(key), _velocity(velocity), _channel(channel), _rel(velocity)
{
}

nlohmann::json Note::toJson(SerializeContext& context)
{
    nlohmann::json json = SequenceItem::toJson(context);
    json["type"] = TYPE;
    json.update(*this);
    return json;
}

// TODO クリップが切り詰めてあるとき
void Note::prepareProcessBuffer(Lane* lane, double begin, double end, double clipBegin, double /*clipEnd*/, double loopBegin, double loopEnd, double oneBeatSec)
{
    ProcessBuffer& processBuffer = lane->_track->_processBuffer;
    double sampleRate = gPreference.sampleRate;
    double noteBegin = clipBegin + _time;
    double noteEnd = noteBegin + _duration;
    int16_t channel = 0;
    if (begin < end)
    {
        if (begin <= noteBegin && noteBegin < end)
        {
            double sampleOffsetDouble = (noteBegin - begin) * oneBeatSec * sampleRate;
            uint32_t sampleOffset = std::round(sampleOffsetDouble);
            processBuffer._eventOut.noteOn(_key, channel, _velocity, sampleOffset);
        }
        if (begin < noteEnd && noteEnd <= end)
        {
            double sampleOffsetDouble = (noteEnd - begin) * oneBeatSec * sampleRate;
            uint32_t sampleOffset = std::round(sampleOffsetDouble);
            processBuffer._eventOut.noteOff(_key, channel, 1.0f, sampleOffset);
        }
    }
    else
    {
        if (begin <= noteBegin && noteBegin < loopEnd)
        {
            double sampleOffsetDouble = (noteBegin - begin) * oneBeatSec * sampleRate;
            uint32_t sampleOffset = std::round(sampleOffsetDouble);
            processBuffer._eventOut.noteOn(_key, channel, _velocity, sampleOffset);
        }
        if (loopBegin <= noteBegin && noteBegin < end)
        {
            double sampleOffsetDouble = (noteBegin - loopBegin) * oneBeatSec * sampleRate;
            uint32_t sampleOffset = std::round(sampleOffsetDouble);
            processBuffer._eventOut.noteOn(_key, channel, _velocity, sampleOffset);
        }
        if (begin < noteEnd && noteEnd <= loopEnd)
        {
            double sampleOffsetDouble = (noteEnd - begin) * oneBeatSec * sampleRate;
            uint32_t sampleOffset = std::round(sampleOffsetDouble);
            processBuffer._eventOut.noteOff(_key, channel, 1.0f, sampleOffset);
        }
        if (loopBegin < noteEnd && noteEnd <= end)
        {
            double sampleOffsetDouble = (noteEnd - loopBegin) * oneBeatSec * sampleRate;
            uint32_t sampleOffset = std::round(sampleOffsetDouble);
            processBuffer._eventOut.noteOff(_key, channel, 1.0f, sampleOffset);
        }
    }
}

void Note::render(const ImVec2& pos1, const ImVec2& pos2, const bool selected)
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImU32 color = selected ? selectedColor(_color) : _color;
    drawList->AddRectFilled(pos1, pos2, color, 2.5f);
}

