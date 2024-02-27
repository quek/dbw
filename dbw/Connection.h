#pragma once
#include <deque>
#include <memory>
#include <vector>
#include "Nameable.h"

class Module;

class Connection : public Nameable {
public:
    Connection(const nlohmann::json& josn);
    Connection(Module* from, int fromIndex, Module* to, int toIndex);
    void deleteFromModule();
    void resolveModuleReference();
    void process(Module* to);
    void setLatency(uint32_t latency);
    virtual nlohmann::json toJson() override;

    Module* _from = nullptr;
    NekoId _fromNekoRef = 0;
    int _fromIndex;

    Module* _to = nullptr;
    NekoId _toNekoRef = 0;
    int _toIndex;

    uint32_t _latency = 0;
    std::vector<std::deque<float>> _dcpBuffer;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Connection, _fromIndex, _toIndex);
};

