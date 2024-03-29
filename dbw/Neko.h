#pragma once
#include <map>
#include <memory>
#include <nlohmann/json.hpp>

class SerializeContext;
typedef uint64_t NekoId;

class Neko {
public:
    Neko();
    Neko(const nlohmann::json& json, SerializeContext& context);
    Neko(const Neko& other);
    virtual ~Neko();
    virtual const NekoId getNekoId() const;
    virtual void setNekoId(NekoId id);
    virtual nlohmann::json toJson(SerializeContext& context);

    static std::map<NekoId, Neko*> idNekoMap;
    template<typename T>
    static T* findByNekoId(NekoId id) {
        auto it = idNekoMap.find(id);
        if (it != idNekoMap.end()) {
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
nlohmann::json renewNekoId(const nlohmann::json& josn);
