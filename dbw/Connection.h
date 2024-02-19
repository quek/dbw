#pragma once
#include <deque>
#include <memory>
#include <vector>
#include "tinyxml2/tinyxml2.h"

class Module;

class Connection {
public:
    Connection(Module* from, int fromIndex, Module* to, int toIndex);
    tinyxml2::XMLElement* toXml(tinyxml2::XMLDocument* doc);
    static std::unique_ptr<Connection> fromXml(tinyxml2::XMLElement* element);
    void resolveModuleReference();
    void process(Module* to);
    void setLatency(uint32_t latency);

    Module* _from;
    uint64_t _fromId = 0;
    int _fromIndex;
 
    Module* _to;
    uint64_t _toId = 0;
    int _toIndex;

    uint32_t _latency = 0;
    std::vector<std::deque<float>> _dcpBuffer;
};

