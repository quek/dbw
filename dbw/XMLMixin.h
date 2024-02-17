#pragma once
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

    template<typename T>
    static T* get(uint64_t id);
private:
    uint64_t _id;
};
