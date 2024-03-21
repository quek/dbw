#pragma once
#include "BuiltinModule.h"

class Fader : public BuiltinModule {
public:
    Fader(const nlohmann::json& json, SerializeContext& context);
    Fader(std::string name, Track* track);
    virtual ~Fader() = default;
    bool process(ProcessBuffer* buffer, int64_t steadyTime) override;
    void renderContent() override;

    virtual nlohmann::json toJson(SerializeContext& context) override;

    float _level = 1.0f;
    float _pan = 0.5f;
    bool _mute = false;
    bool _solo = false;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Fader, _level, _pan, _mute, _solo);

private:
    float linearToGainRatio(float linearValue);
    float gainRatioToDB(float gainRatio);
    float gainRatioToLinear(float gainRatio);

    float _peakValue = 0.0f;
    float _peakValueHold = 0.0f;
    int _peakSampleElapsed = 0;
};

