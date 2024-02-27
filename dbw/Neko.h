#pragma once
#include <map>
#include <memory>
#include <nlohmann/json.hpp>

typedef uint64_t NekoId;

class Neko {
public:
    Neko();
    Neko(const nlohmann::json& json);
    Neko(const Neko& other);
    virtual ~Neko();
    virtual const NekoId nekoId() const;
    virtual void setNekoId(NekoId id);
    virtual nlohmann::json toJson();

    static std::map<NekoId, Neko*> nekoIdMap;
    template<typename T>
    static T* findByNekoId(NekoId id) {
        auto it = nekoIdMap.find(id);
        if (it != nekoIdMap.end()) {
            return dynamic_cast<T*>(it->second);
        }
        return nullptr;
    }

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Neko, _nekoId);

private:
    void setNewNekoId();

    NekoId _nekoId;
};

nlohmann::json eraseNekoId(const nlohmann::json& josn);
