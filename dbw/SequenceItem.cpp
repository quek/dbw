#include "SequenceItem.h"
#include "Note.h"

SequenceItem* SequenceItem::create(const nlohmann::json& json) {
    // TODO
    return new Note(json);
}
