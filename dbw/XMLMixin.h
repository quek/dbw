#pragma once
#include <map>
#include <memory>
#include "tinyxml2/tinyxml2.h"


class XMLMixin {
public:
    XMLMixin();
    XMLMixin(const XMLMixin& other);
    virtual ~XMLMixin();
    virtual tinyxml2::XMLElement* toXml(tinyxml2::XMLDocument* doc) = 0;
    virtual const uint64_t xmlId() const;
    virtual void setXMLId(uint64_t id);

    static std::map<uint64_t, XMLMixin*> idMap;
    template<typename T>
    static T* findByXMLId(uint64_t id) {
        auto it = idMap.find(id);
        if (it != idMap.end()) {
            return dynamic_cast<T*>(it->second);
        }
        return nullptr;
    }

private:
    uint64_t _xmlId;
};
