#include "Sequence.h"
#include <map>

static std::map<uint64_t, std::weak_ptr<Sequence>> xmlIdSequenceMap;
int Sequence::_no = 0;

Sequence::Sequence(double duration) : Nameable("Seq" + std::to_string(++_no)), _duration(duration) {
}

std::shared_ptr<Sequence> Sequence::create(double duration, uint64_t id) {
    std::shared_ptr<Sequence> sequence(new Sequence(duration));
    if (id != 0) {
        sequence->setXMLId(id);
    }
    xmlIdSequenceMap[sequence->xmlId()] = sequence;
    return sequence;
}

Sequence::~Sequence() {
    xmlIdSequenceMap.erase(xmlId());
}

tinyxml2::XMLElement* Sequence::toXml(tinyxml2::XMLDocument* doc) {
    auto element = doc->NewElement("Notes");
    element->SetAttribute("id", xmlId());
    element->SetAttribute("name", _name.c_str());
    for (auto& note : _notes) {
        auto noteElement = note->toXml(doc);
        element->InsertEndChild(noteElement);
    }
    return element;
}

std::shared_ptr<Sequence> Sequence::fromXml(tinyxml2::XMLElement* element) {
    uint64_t id = 0;
    element->QueryUnsigned64Attribute("id", &id);

    auto it = xmlIdSequenceMap.find(id);
    if (it != xmlIdSequenceMap.end()) {
        auto sequence = it->second.lock();
        if (sequence) {
            return sequence;
        }
    }

    double duration = 16.0;
    element->QueryDoubleAttribute("duration", &duration);
    std::shared_ptr<Sequence> sequence = Sequence::create(duration, id);
    sequence->_name = element->Attribute("name");
    for (auto noteElement = element->FirstChildElement("Note");
         noteElement != nullptr;
         noteElement = noteElement->NextSiblingElement("Note")) {
        sequence->_notes.emplace_back(Note::fromXml(noteElement));
    }
    return sequence;
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
