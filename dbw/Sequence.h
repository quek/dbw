#pragma once
#include <map>
#include <memory>
#include <vector>
#include "Command.h"
#include "Nameable.h"
#include "Neko.h"
#include "SequenceItem.h"

class Composer;

class Sequence : public Nameable {
public:
    inline static const char* TYPE = "sequence";
    static std::shared_ptr<Sequence>create(double duration = 16.0, NekoId id = 0) {
        std::shared_ptr<Sequence> sequence(new Sequence(duration));
        if (id != 0) {
            sequence->setNekoId(id);
        }
        nekoIdSequenceMap[sequence->getNekoId()] = sequence;
        return sequence;
    }
    static std::shared_ptr<Sequence>create(const nlohmann::json& json) {
        if (json.contains("sequenceId")) {
            auto& p = nekoIdSequenceMap[json["sequenceId"].template get<NekoId>()];
            auto sequence = p.lock();
            if (sequence) {
                return sequence;
            }
        }
        std::shared_ptr<Sequence> sequence(new Sequence(json));
        nekoIdSequenceMap[sequence->getNekoId()] = sequence;
        return sequence;
    }
    virtual ~Sequence() {
        nekoIdSequenceMap.erase(getNekoId());
    }
    virtual nlohmann::json toJson() {
        nlohmann::json json = Nameable::toJson();
        json["sequenceId"] = getNekoId();
        json["type"] = TYPE;
        json["_duration"] = _duration;

        nlohmann::json notes = nlohmann::json::array();
        for (const auto& note : _notes) {
            notes.emplace_back(note->toJson());
        }
        json["_notes"] = notes;

        return json;
    }


    std::vector<std::unique_ptr<SequenceItem>> _notes;
    double _duration;

    static int _no;
    static std::map<NekoId, std::weak_ptr<Sequence>> nekoIdSequenceMap;

private:
    Sequence() = default;
    Sequence(const nlohmann::json& json) : Nameable(json) {
        _duration = json["_duration"];
        for (const auto& x : json["_notes"]) {
            _notes.emplace_back(SequenceItem::create(x));
        }
    }
    Sequence(double duration) : Nameable("Seq" + std::to_string(++_no)), _duration(duration) {
    }

};

