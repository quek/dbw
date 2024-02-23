#pragma once
#include "BuiltinModule.h"

class Fader : public BuiltinModule {
public:
    Fader(const nlohmann::json& json);
    Fader(std::string name, Track* track);
    virtual ~Fader() = default;
    bool process(ProcessBuffer* buffer, int64_t steadyTime) override;
    void renderContent() override;

    virtual nlohmann::json toJson() override;

    float _level = 1.0f;
    float _pan = 0.5f;
    bool _mute = false;
    bool _solo = false;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Fader, _level, _pan, _mute, _solo);
};

