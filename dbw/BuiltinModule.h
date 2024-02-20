#pragma once
#include "Module.h"


class BuiltinModule : public Module {
public:
    BuiltinModule(std::string name, Track* track);
    virtual ~BuiltinModule() = default;
    virtual void loadParameters(tinyxml2::XMLElement* element) = 0;

    static BuiltinModule* create(std::string& id);
};

