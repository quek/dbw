#pragma once
#include <cstdint>
#include "Thing.h"
#include "XMLMixin.h"

class Note : public Thing, public XMLMixin {
public:
    Note(double time = 0.0f, double duration = 1.0f, int16_t key = 64, double velocity = 0.8, int16_t channel = 0);
    virtual ~Note() = default;
    tinyxml2::XMLElement* toXml(tinyxml2::XMLDocument* doc) override;
    static std::unique_ptr<Note> fromXml(tinyxml2::XMLElement* element);

    int16_t _channel;
    int16_t _key;
    double _velocity;
    double _rel;
};
