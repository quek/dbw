#pragma once
#include "BuiltinModule.h"

class Fader : public BuiltinModule {
public:
    Fader(std::string name, Track* track);
    virtual ~Fader() = default;
    virtual tinyxml2::XMLElement* dawProject(tinyxml2::XMLDocument* doc) override;
    virtual void loadParameters(tinyxml2::XMLElement* element) override;
    bool process(ProcessBuffer* buffer, int64_t steadyTime) override;
    void renderContent() override;
private:
    float _level;
};

