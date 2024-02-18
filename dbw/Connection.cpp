#include "Connection.h"
#include "Module.h"

Connection::Connection(Module* from, int fromIndex, Module* to, int toIndex) :
    _from(from), _fromIndex(fromIndex), _to(to), _toIndex(toIndex) {
}

tinyxml2::XMLElement* Connection::toXml(tinyxml2::XMLDocument* doc) {
    auto element = doc->NewElement("Connection");
    element->SetAttribute("from", _from->xmlId());
    element->SetAttribute("fromIndex", _fromIndex);
    element->SetAttribute("to", _to->xmlId());
    element->SetAttribute("toIndex", _toIndex);
    return element;
}

std::unique_ptr<Connection> Connection::fromXml(tinyxml2::XMLElement* element) {
    int fromIndex;
    element->QueryIntAttribute("fromIndex", &fromIndex);
    int toIndex;
    element->QueryIntAttribute("toIndex", &toIndex);
    std::unique_ptr<Connection> connection(new Connection(nullptr, fromIndex, nullptr, toIndex));
    element->QueryUnsigned64Attribute("from", &connection->_fromId);
    element->QueryUnsigned64Attribute("to", &connection->_toId);
    return connection;
}

void Connection::resolveModuleReference() {
    _from = XMLMixin::findByXMLId<Module>(_fromId);
    _to = XMLMixin::findByXMLId<Module>(_toId);
}
