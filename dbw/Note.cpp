#include "Note.h"

Note::Note(double time, double duration, int16_t key, double velocity, int16_t channel) :
    Thing(time, duration), _key(key), _velocity(velocity), _channel(channel), _rel(velocity) {
}

nlohmann::json Note::toJson() {
    nlohmann::json json = Neko::toJson();
    json["type"] = TYPE;
    json.update(*this);
    return json;
}
