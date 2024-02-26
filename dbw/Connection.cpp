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

void Connection::deleteFromModule() {
    std::erase_if(_to->_connections, [this](auto& x) { return x->_from == _from; });
    std::erase_if(_from->_connections, [this](auto& x) { return x->_to == _to; });
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
        bool xConstantp = _from->_track->_processBuffer._out.at(_fromIndex)._constantp[channel];
        bool yConstantp = _to->_track->_processBuffer._in.at(_toIndex)._constantp[channel];
        auto& x = _from->_track->_processBuffer._out.at(_fromIndex).buffer32()[channel];
        auto& y = to->_track->_processBuffer._in.at(_toIndex).buffer32()[channel];
        auto y0 = y[0];
        for (size_t i = 0; i < gPreference.bufferSize; ++i) {
            size_t xi = i;
            if (xConstantp) {
                xi = 0;
            }
            if (_latency == 0) {
                if (yConstantp) {
                    y[i] = y0 + x[xi];
                } else {
                    y[i] += x[xi];
                }
            } else {
                auto& dcp = _dcpBuffer[channel];
                dcp.push_back(x[xi]);
                if (yConstantp) {
                    y[i] = y0 + dcp.front();
                } else {
                    y[i] += dcp.front();
                }
                dcp.pop_front();
            }
        }
        to->_track->_processBuffer._in.at(_toIndex)._constantp[channel] = false;
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
    json["_fromId"] = _from->nekoId();
    json["_toId"] = _to->nekoId();
    return json;
}
