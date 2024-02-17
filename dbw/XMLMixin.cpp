#include "XmlMixin.h"
#include <atomic>
#include <cstdint>
#include <map>

static std::atomic<uint64_t> idSeq(0);
//static std::map<uint64_t, XMLMixin*> idMap;

XMLMixin::XMLMixin() :_id(++idSeq) {
    //idMap[_id] = this;
}

XMLMixin::XMLMixin(const XMLMixin& other) {
    _id = ++idSeq;
    //idMap[_id] = this;
}

XMLMixin::~XMLMixin() {
    //idMap.erase(_id);
}

const uint64_t XMLMixin::xmlId() const {
    return _id;
}

void XMLMixin::setXMLId(uint64_t id) {
    _id = id;
}

//template<typename T>
//T* XMLMixin::get(uint64_t id) {
//    auto it = idMap.find(id);
//    if (it != idMap.end()) {
//        return dynamic_cast<T*>(it->second.get());
//    }
//    return nullptr;
//}
