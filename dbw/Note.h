#pragma once
#include <cstdint>
#include "Thing.h"
#include "Neko.h"

class Note : public Thing, public Neko {
public:
    inline static const char* TYPE = "note";
    Note(double time = 0.0f, double duration = 1.0f, int16_t key = 64, double velocity = 0.8, int16_t channel = 0);
    virtual ~Note() = default;
    tinyxml2::XMLElement* toXml(tinyxml2::XMLDocument* doc) override;
    static std::unique_ptr<Note> fromXml(tinyxml2::XMLElement* element);
    virtual nlohmann::json toJson() override;

    int16_t _channel;
    int16_t _key;
    double _velocity;
    double _rel;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Note, _time, _duration, _channel, _key, _velocity, _rel);
};
