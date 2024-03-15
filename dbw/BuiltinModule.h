#pragma once
#include "Module.h"

class BuiltinModule : public Module {
public:
    inline static const char* TYPE = "builtin";
    BuiltinModule(const nlohmann::json& json, SerializeContext& context);
    BuiltinModule(std::string name, Track* track);
    virtual ~BuiltinModule() = default;
    virtual nlohmann::json toJson(SerializeContext& context) override;

    static BuiltinModule* create(std::string& id);
};

