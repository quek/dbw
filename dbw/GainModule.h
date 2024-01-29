#pragma once
#include "BuiltinModule.h"

class GainModule : public BuiltinModule {
public:
    GainModule(std::string name, Track* track);
    virtual ~GainModule() = default;
    virtual tinyxml2::XMLElement* dawProject(tinyxml2::XMLDocument* doc) override;
    virtual void loadParameters(tinyxml2::XMLElement* element) override;
    bool process(ProcessBuffer* buffer, int64_t steadyTime) override;
    void render() override;
private:
    float _gain;
};

