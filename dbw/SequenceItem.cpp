#include "SequenceItem.h"
#include "Audio.h"
#include "AutomationPoint.h"
#include "Note.h"

SequenceItem* SequenceItem::create(const nlohmann::json& json, SerializeContext& context)
{
    if (json["type"] == Note::TYPE)
    {
        return new Note(json, context);
    }
    if (json["type"] == AutomationPoint::TYPE)
    {
        return new AutomationPoint(json, context);
    }
    if (json["type"] == Audio::TYPE)
    {
        return new Audio(json, context);
    }
    return nullptr;
}

SequenceItem::SequenceItem(const nlohmann::json& json, SerializeContext& context) : Nameable(json, context)
{
    _time = json["_time"];
    _duration = json["_duration"];
}

SequenceItem::SequenceItem(double time, double duration) : _time(time), _duration(duration)
{
}

void SequenceItem::addTo(std::vector<std::unique_ptr<SequenceItem>>& items)
{
    items.emplace_back(this);
}

void SequenceItem::render(const ImVec2&, const ImVec2&, const ImVec2&, const ImVec2&, const bool)
{
}

void SequenceItem::setTime(double time)
{
    _time = std::max(0.0, time);
}

nlohmann::json SequenceItem::toJson(SerializeContext& context)
{
    nlohmann::json json = Nameable::toJson(context);
    json["_time"] = _time;
    json["_duration"] = _duration;
    return json;
}
