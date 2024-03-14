#include "Clip.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "AudioClip.h"
#include "AutomationClip.h"
#include "NoteClip.h"
#include "PianoRollWindow.h"

Clip* Clip::create(const nlohmann::json& json) {
    if (json["type"] == NoteClip::TYPE) {
        return new NoteClip(json);
    }
    if (json["type"] == AutomationClip::TYPE) {
        return new AutomationClip(json);
    }
    if (json["type"] == AudioClip::TYPE) {
        return new AudioClip(json);
    }
    assert(false);
    return nullptr;
}

Clip::Clip(const nlohmann::json& json) : Nameable(json) {
    _time = json["_time"];
    _duration = json["_duration"];
    _sequence = Sequence::create(json["_sequence"]);
}

Clip::Clip(double time, double duration) :
    _time(time), _duration(duration), _sequence(Sequence::create()) {
}

std::string Clip::name() const {
    std::string name = (_sequence.use_count() > 1 ? "∞" : "") + _sequence->_name;
    return name;
}

void Clip::prepareProcessBuffer(Lane* lane, double begin, double end, double loopBegin, double loopEnd, double oneBeatSec) {
    double clipBegin = _time;
    double clipEnd = _duration + clipBegin;
    if (begin < end) {
        if (clipBegin < end && begin <= clipEnd) {
            // ok
        } else {
            return;
        }
    } else {
        if (clipBegin < end && loopBegin <= clipEnd || begin <= clipEnd && clipBegin < loopEnd) {
            // ok
        } else {
            return;
        }
    }
    for (auto& item : _sequence->getItems()) {
        item->prepareProcessBuffer(lane, begin, end, clipBegin, clipEnd, loopBegin, loopEnd, oneBeatSec);
    }
}

nlohmann::json Clip::toJson() {
    nlohmann::json json = Nameable::toJson();
    json["_sequence"] = _sequence->toJson();
    json["_time"] = _time;
    json["_duration"] = _duration;
    return json;
}
