#include "XmlMixin.h"
#include <atomic>
#include <cstdint>

std::map<uint64_t, XMLMixin*> XMLMixin::idMap;

static std::atomic<uint64_t> idSeq(0);

XMLMixin::XMLMixin() :_xmlId(++idSeq) {
    //idMap[_id] = this;
}

XMLMixin::XMLMixin(const XMLMixin&) {
    _xmlId = ++idSeq;
    idMap[_xmlId] = this;
}

XMLMixin::~XMLMixin() {
    idMap.erase(_xmlId);
}

const uint64_t XMLMixin::xmlId() const {
    return _xmlId;
}

void XMLMixin::setXMLId(uint64_t id) {
    idMap.erase(_xmlId);
    _xmlId = id;
    idMap[_xmlId] = this;
    if (idSeq.load() < id) {
        idSeq.store(id);
    }
}
