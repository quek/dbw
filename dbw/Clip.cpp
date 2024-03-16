#include "Clip.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "AudioClip.h"
#include "AutomationClip.h"
#include "NoteClip.h"
#include "PianoRollWindow.h"

Clip* Clip::create(const nlohmann::json& json, SerializeContext& context) {
    if (json["type"] == NoteClip::TYPE) {
        return new NoteClip(json, context);
    }
    if (json["type"] == AutomationClip::TYPE) {
        return new AutomationClip(json, context);
    }
    if (json["type"] == AudioClip::TYPE) {
        return new AudioClip(json, context);
    }
    assert(false);
    return nullptr;
}

Clip::Clip(const nlohmann::json& json, SerializeContext& context) : Nameable(json, context) {
    _time = json["_time"];
    _duration = json["_duration"];
    _sequence = Sequence::create(json["_sequence"], context);
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

void Clip::render(const ImVec2& pos1, const ImVec2& pos2, const bool selected)
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImU32 color = selected? selectedColor(_color) : _color;
    drawList->AddRectFilled(pos1, pos2, color, 2.5f);
}

nlohmann::json Clip::toJson(SerializeContext& context) {
    nlohmann::json json = Nameable::toJson(context);
    json["_sequence"] = _sequence->toJson(context);
    json["_time"] = _time;
    json["_duration"] = _duration;
    return json;
}
