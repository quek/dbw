#include "SequenceItem.h"
#include "AutomationPoint.h"
#include "Note.h"

SequenceItem* SequenceItem::create(const nlohmann::json& json) {
    if (json["type"] == Note::TYPE) {
        return new Note(json);
    }
    if (json["type"] == AutomationPoint::TYPE) {
        return new AutomationPoint(json);
    }
    return nullptr;
}

SequenceItem::SequenceItem(double time, double duration) : _time(time), _duration(duration) {
}

