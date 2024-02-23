#include "Connection.h"
#include "Config.h"
#include "Module.h"
#include "Track.h"

Connection::Connection(const nlohmann::json& json) : Nameable(json) {
    _fromId = json["_fromId"];
    _fromIndex = json["_fromIndex"];
    _toId = json["_toId"];
    _toIndex = json["_toIndex"];
}

Connection::Connection(Module* from, int fromIndex, Module* to, int toIndex) :
    _from(from), _fromIndex(fromIndex), _to(to), _toIndex(toIndex) {
    setLatency(0);
}

void Connection::resolveModuleReference() {
    _from = Neko::findByNekoId<Module>(_fromId);
    _to = Neko::findByNekoId<Module>(_toId);
}

void Connection::process(Module* to) {
    if (_to != to) {
        return;
    }
    if (!_from->isStarting()) {
        return;
    }
    for (int channel = 0; channel < 2; ++channel) {
        auto& x = _from->_track->_processBuffer._out.at(_fromIndex).buffer32()[channel];
        auto& y = to->_track->_processBuffer._in.at(_toIndex).buffer32()[channel];
        for (size_t i = 0; i < gPreference.bufferSize; ++i) {
            if (_latency == 0) {
                y[i] = x[i];
            } else {
                auto& dcp = _dcpBuffer[channel];
                dcp.push_back(x[i]);
                y[i] = dcp.front();
                dcp.pop_front();
            }
        }
    }
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

nlohmann::json Connection::toJson() {
    nlohmann::json json = Nameable::toJson();
    json.update(*this);
    return json;
}
