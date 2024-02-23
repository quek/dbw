#include "Sequence.h"
#include <map>

static std::map<uint64_t, std::weak_ptr<Sequence>> nekoIdSequenceMap;
int Sequence::_no = 0;

Sequence::Sequence(double duration) : Nameable("Seq" + std::to_string(++_no)), _duration(duration) {
}

std::shared_ptr<Sequence> Sequence::create(double duration, uint64_t id) {
    std::shared_ptr<Sequence> sequence(new Sequence(duration));
    if (id != 0) {
        sequence->setNekoId(id);
    }
    nekoIdSequenceMap[sequence->nekoId()] = sequence;
    return sequence;
}

std::shared_ptr<Sequence> Sequence::create(const nlohmann::json& json) {
    if (json.contains("sequenceId")) {
        auto& p = nekoIdSequenceMap[json["sequenceId"].template get<uint64_t>()];
        auto sequence = p.lock();
        if (sequence) {
            return sequence;
        }
    }
    std::shared_ptr<Sequence> sequence(new Sequence(json));
    nekoIdSequenceMap[sequence->nekoId()] = sequence;
    return sequence;
}

Sequence::Sequence(const nlohmann::json& json) : Nameable(json) {
    _duration = json["_duration"];
    for (const auto& x : json["_notes"]) {
        _notes.emplace_back(new Note(x));
    }
}

Sequence::~Sequence() {
    nekoIdSequenceMap.erase(nekoId());
}

nlohmann::json Sequence::toJson() {
    nlohmann::json json = Nameable::toJson();
    json["sequenceId"] = nekoId();
    json["type"] = TYPE;
    json["_duration"] = _duration;

    nlohmann::json notes = nlohmann::json::array();
    for (const auto& note : _notes) {
        notes.emplace_back(note->toJson());
    }
    json["_notes"] = notes;

    return json;
}

DeleteNoteCommand::DeleteNoteCommand(Sequence* sequence, Note* note, bool undoable) : Command(undoable), _sequence(sequence), _note(note) {
}

void DeleteNoteCommand::execute(Composer* /* composer */) {
    for (auto note = _sequence->_notes.begin(); note != _sequence->_notes.end(); ++note) {
        if ((*note).get() == _note) {
            _noteForUndo = std::move(*note);
            _sequence->_notes.erase(note);
            break;
        }
    }
}

void DeleteNoteCommand::undo(Composer* /*composer*/) {
    _sequence->_notes.push_back(std::move(_noteForUndo));
}
