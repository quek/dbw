#pragma once
#include <map>
#include <memory>
#include <vector>
#include "Command.h"
#include "Nameable.h"
#include "Neko.h"
#include "SequenceItem.h"

class Composer;

class Sequence : public Nameable
{
public:
    static std::shared_ptr<Sequence>create(double duration = 16.0, NekoId id = 0);
    static std::shared_ptr<Sequence>create(const nlohmann::json& json, SerializeContext& context);
    virtual ~Sequence();
    virtual void addItem(SequenceItem* item);
    virtual void deleteItem(SequenceItem* item);
    double durationGet() const { return _duration; }
    void durationSet(double duration) { _duration = duration; }
    std::vector<std::unique_ptr<SequenceItem>>& getItems();
    nlohmann::json toJson(SerializeContext& context) override;



    static int _no;
    static std::map<NekoId, std::weak_ptr<Sequence>> nekoIdSequenceMap;

private:
    Sequence() = default;
    Sequence(const nlohmann::json& json, SerializeContext& context);
    Sequence(double duration);

    double _duration;
    std::vector<std::unique_ptr<SequenceItem>> _items;
};

