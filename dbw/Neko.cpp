#include "Neko.h"
#include <atomic>
#include <cstdint>

std::map<uint64_t, Neko*> Neko::nekoIdMap;

static std::atomic<uint64_t> idSeq(0);

Neko::Neko() {
    setNewNekoId();
}

Neko::Neko(const Neko&) {
    setNewNekoId();
}

Neko::~Neko() {
    nekoIdMap.erase(_nekoId);
}

const uint64_t Neko::xmlId() const {
    return _nekoId;
}

void Neko::setNekoId(uint64_t id) {
    nekoIdMap.erase(_nekoId);
    _nekoId = id;
    nekoIdMap[_nekoId] = this;
}

nlohmann::json Neko::toJson() {
    return *this;
}

void Neko::setNewNekoId() {
    while (true) {
        _nekoId = ++idSeq;
        if (nekoIdMap.contains(_nekoId)) {
            continue;
        }
        nekoIdMap[_nekoId] = this;
        break;
    }
}
