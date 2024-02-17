#pragma once
#include "BuiltinModule.h"

class Fader : public BuiltinModule {
public:
    Fader(std::string name, Track* track);
    virtual ~Fader() = default;
    virtual tinyxml2::XMLElement* toXml(tinyxml2::XMLDocument* doc) override;
    virtual void loadParameters(tinyxml2::XMLElement* element) override;
    bool process(ProcessBuffer* buffer, int64_t steadyTime) override;
    void renderContent() override;

    float _level = 1.0f;
    float _pan = 0.5f;
    bool _mute = false;
    bool _solo = false;
};

