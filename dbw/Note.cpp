#include "Note.h"

Note::Note(double time, double duration, int16_t key, double velocity, int16_t channel) :
    Thing(time, duration), _key(key), _velocity(velocity), _channel(channel), _rel(velocity) {
}

tinyxml2::XMLElement* Note::toXml(tinyxml2::XMLDocument* doc) {
    auto element = doc->NewElement("Note");
    element->SetAttribute("key", _key);
    element->SetAttribute("time", _time);
    element->SetAttribute("duration", _duration);
    element->SetAttribute("vel", _velocity);
    element->SetAttribute("rel", _rel);
    return element;
}

std::unique_ptr<Note> Note::fromXml(tinyxml2::XMLElement* element) {
    std::unique_ptr<Note> note(new Note());
    element->QueryDoubleAttribute("time", &note->_time);
    element->QueryDoubleAttribute("duration", &note->_duration);
    int intValue;
    element->QueryIntAttribute("channel", &intValue);
    note->_channel = intValue;
    element->QueryIntAttribute("key", &intValue);
    note->_key = intValue;
    element->QueryDoubleAttribute("vel", &note->_velocity);
    element->QueryDoubleAttribute("rel", &note->_rel);
    return note;
}
