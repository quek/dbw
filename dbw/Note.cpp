#include "Note.h"
#include "Config.h"
#include "ProcessBuffer.h"

Note::Note(const nlohmann::json& json) : SequenceItem(json) {
    _channel = json["_channel"];
    _key = json["_key"];
    _velocity = json["_velocity"];
    _rel = json["_rel"];
}

Note::Note(double time, double duration, int16_t key, double velocity, int16_t channel) :
    SequenceItem(time, duration), _key(key), _velocity(velocity), _channel(channel), _rel(velocity) {
}

nlohmann::json Note::toJson() {
    nlohmann::json json = SequenceItem::toJson();
    json["type"] = TYPE;
    json.update(*this);
    return json;
}

void Note::prepareProcessBuffer(ProcessBuffer* processBuffer, double begin, double end, double clipTime, double clipDuration, double oneBeatSec) {
    double sampleRate = gPreference.sampleRate;
    double noteTime = clipTime + _time;
    if ((begin <= noteTime && noteTime < end) || (end < begin && (begin <= noteTime || noteTime < end))) {
        int16_t channel = 0;
        uint32_t sampleOffsetDouble = 0;
        if (begin < end) {
            sampleOffsetDouble = (noteTime - begin) * oneBeatSec * sampleRate;
        } else {
            sampleOffsetDouble = (noteTime + clipDuration - begin) * oneBeatSec * sampleRate;
        }
        uint32_t sampleOffset = std::round(sampleOffsetDouble);
        processBuffer->_eventOut.noteOn(_key, channel, _velocity, sampleOffset);
    }
    noteTime = noteTime + _duration;
    if ((begin <= noteTime && noteTime < end) || (end < begin && (begin <= noteTime || noteTime < end))) {
        int16_t channel = 0;
        uint32_t sampleOffsetDouble = 0;
        if (begin < end) {
            sampleOffsetDouble = (noteTime - begin) * oneBeatSec * sampleRate;
        } else {
            sampleOffsetDouble = (noteTime + clipDuration - begin) * oneBeatSec * sampleRate;
        }
        uint32_t sampleOffset = std::round(sampleOffsetDouble);
        processBuffer->_eventOut.noteOff(_key, channel, 1.0f, sampleOffset);
    }
}

