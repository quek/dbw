#include "Note.h"
#include "Config.h"
#include "ProcessBuffer.h"

Note::Note(double time, double duration, int16_t key, double velocity, int16_t channel) :
    _time(time), _duration(duration), _key(key), _velocity(velocity), _channel(channel), _rel(velocity) {
}

nlohmann::json Note::toJson() {
    nlohmann::json json = SequenceItem::toJson();
    json["type"] = TYPE;
    json.update(*this);
    return json;
}

void Note::prepareProcessBuffer(ProcessBuffer* processBuffer, double begin, double end, double clipTime, double clipDuration, double oneBeatSec) {
    double sampleRate = gPreference.sampleRate;
    if (begin < clipTime + clipDuration && clipTime < end) {
        double noteTime = clipTime + _time;
        if (begin <= noteTime && noteTime < end) {
            int16_t channel = 0;
            uint32_t sampleOffsetDouble = (noteTime - begin) * oneBeatSec * sampleRate;
            uint32_t sampleOffset = std::round(sampleOffsetDouble);
            processBuffer->_eventOut.noteOn(_key, channel, _velocity, sampleOffset);
        }
        double noteDuration = noteTime + _duration;
        if (begin <= noteDuration && noteDuration < end) {
            int16_t channel = 0;
            uint32_t sampleOffsetDouble = (noteDuration - begin) * oneBeatSec * sampleRate;
            uint32_t sampleOffset = std::round(sampleOffsetDouble);
            processBuffer->_eventOut.noteOff(_key, channel, 1.0f, sampleOffset);
        }
    }
}

void Note::prepareProcessBuffer(ProcessBuffer* processBuffer, double begin, double end, double sequenceDuration, double oneBeatSec) {
    double sampleRate = gPreference.sampleRate;
    if ((begin <= _time && _time < end) || (end < begin && (begin <= _time || _time < end))) {
        int16_t channel = 0;
        uint32_t sampleOffsetDouble = 0;
        if (begin < end) {
            sampleOffsetDouble = (_time - begin) * oneBeatSec * sampleRate;
        } else {
            sampleOffsetDouble = (_time + sequenceDuration - begin) * oneBeatSec * sampleRate;
        }
        uint32_t sampleOffset = std::round(sampleOffsetDouble);
        processBuffer->_eventOut.noteOn(_key, channel, _velocity, sampleOffset);
    }
    _time = _time + _duration;
    if ((begin <= _time && _time < end) || (end < begin && (begin <= _time || _time < end))) {
        int16_t channel = 0;
        uint32_t sampleOffsetDouble = 0;
        if (begin < end) {
            sampleOffsetDouble = (_time - begin) * oneBeatSec * sampleRate;
        } else {
            sampleOffsetDouble = (_time + sequenceDuration - begin) * oneBeatSec * sampleRate;
        }
        uint32_t sampleOffset = std::round(sampleOffsetDouble);
        processBuffer->_eventOut.noteOff(_key, channel, 1.0f, sampleOffset);
    }
}

