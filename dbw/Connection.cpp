#include "Connection.h"
#include "Module.h"
#include "Track.h"

Connection::Connection(Module* from, int fromIndex, Module* to, int toIndex) :
    _from(from), _fromIndex(fromIndex), _to(to), _toIndex(toIndex) {
    setLatency(0);
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

void Connection::process(Module* to) {
    if (_to != to) {
        return;
    }
    if (!_from->isStarting()) {
        return;
    }
    _from->_track->_processBuffer._out.at(_fromIndex).copyTo(to->_track->_processBuffer._in.at(_toIndex));
}

void Connection::setLatency(uint32_t latency) {
    _latency = latency;
    if (_latency == 0) {
        return;
    }
    _dcpBuffer.clear();
    for (int i = 0; i < 2; ++i) {
        _dcpBuffer.emplace_back(latency, 0.0f);
    }
}
