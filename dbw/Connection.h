#pragma once
#include <deque>
#include <memory>
#include <vector>
#include "Nameable.h"

class Module;

class Connection : public Nameable
{
public:
    Connection(const nlohmann::json& json, SerializeContext& context);
    Connection(Module* from, int fromIndex, Module* to, int toIndex, bool post = true);
    void resolveModuleReference();
    void process(Module* to);
    std::string scLabel();
    void setLatency(uint32_t latency);
    virtual nlohmann::json toJson(SerializeContext& context) override;

    Module* _from = nullptr;
    NekoId _fromNekoRef = 0;
    int _fromIndex;

    Module* _to = nullptr;
    NekoId _toNekoRef = 0;
    int _toIndex;

    bool _post = true;

    uint32_t _latency = 0;
    std::vector<std::deque<float>> _dcpBuffer;
};

