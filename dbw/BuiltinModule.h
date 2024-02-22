#pragma once
#include "Module.h"

class BuiltinModule : public Module {
public:
    inline static const char* TYPE = "builtin";
    BuiltinModule(const nlohmann::json& json);
    BuiltinModule(std::string name, Track* track);
    virtual ~BuiltinModule() = default;
    virtual void loadParameters(tinyxml2::XMLElement* element) = 0;
    virtual nlohmann::json toJson() override;

    static BuiltinModule* create(std::string& id);
};

