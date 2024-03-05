#include "Clip.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "PianoRollWindow.h"

Clip::Clip(const nlohmann::json& json) : Nameable(json) {
    _time = json["_time"];
    _duration = json["_duration"];
}

Clip::Clip(double time, double duration) :
    _time(time), _duration(duration){
}

std::string Clip::name() const {
    std::string name = (_sequence.use_count() > 1 ? "∞" : "") + _sequence->_name;
    return name;
}

nlohmann::json Clip::toJson() {
    nlohmann::json json = Nameable::toJson();
    json["type"] = TYPE;
    json["_sequence"] = _sequence->toJson();
    json["_time"] = _time;
    json["_duration"] = _duration;
    return json;
}
