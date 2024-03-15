#include "Sequence.h"
#include <map>
#include "SerializeContext.h"

int Sequence::_no = 0;
std::map<NekoId, std::weak_ptr<Sequence>> Sequence::nekoIdSequenceMap;

std::shared_ptr<Sequence> Sequence::create(double duration, NekoId id)
{
    std::shared_ptr<Sequence> sequence(new Sequence(duration));
    if (id != 0)
    {
        sequence->setNekoId(id);
    }
    nekoIdSequenceMap[sequence->getNekoId()] = sequence;
    return sequence;
}

std::shared_ptr<Sequence> Sequence::create(const nlohmann::json& json, SerializeContext& context)
{
    if (json.contains("sequenceId"))
    {
        auto& p = nekoIdSequenceMap[json["sequenceId"].template get<NekoId>()];
        auto sequence = p.lock();
        if (sequence)
        {
            return sequence;
        }
    }
    std::shared_ptr<Sequence> sequence(new Sequence(json, context));
    nekoIdSequenceMap[sequence->getNekoId()] = sequence;
    return sequence;
}

Sequence::~Sequence()
{
    nekoIdSequenceMap.erase(getNekoId());
}

void Sequence::addItem(SequenceItem* item)
{
    item->addTo(_items);
}

void Sequence::deleteItem(SequenceItem* item)
{
    std::erase_if(_items, [item](const auto& x) { return x.get() == item; });
}

std::vector<std::unique_ptr<SequenceItem>>& Sequence::getItems()
{
    return _items;
}

nlohmann::json Sequence::toJson(SerializeContext& context)
{
    nlohmann::json json;
    if (context.findNeko(getNekoId()))
    {
        json = Neko::toJson(context);
    }
    else
    {
        json = Nameable::toJson(context);
        json["_duration"] = _duration;

        nlohmann::json notes = nlohmann::json::array();
        for (const auto& note : _items)
        {
            notes.emplace_back(note->toJson(context));
        }
        json["_items"] = notes;
    }
    json["sequenceId"] = getNekoId();
    json["type"] = TYPE;

    return json;
}

Sequence::Sequence(const nlohmann::json& json, SerializeContext& context) : Nameable(json, context)
{
    _duration = json["_duration"];
    for (const auto& x : json["_items"])
    {
        addItem(SequenceItem::create(x, context));
    }
}

Sequence::Sequence(double duration) : Nameable("Seq" + std::to_string(++_no)), _duration(duration)
{
}
