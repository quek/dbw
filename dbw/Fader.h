#pragma once
#include "BuiltinModule.h"

class Fader : public BuiltinModule {
public:
    const float ZERO_DB_NORMALIZED_VALUE = 0.7f;

    Fader(const nlohmann::json& json, SerializeContext& context);
    Fader(std::string name, Track* track);
    virtual ~Fader() = default;
    bool process(ProcessBuffer* buffer, int64_t steadyTime) override;
    void render(float width = 0.0f, float height = 0.0f) override;

    virtual nlohmann::json toJson(SerializeContext& context) override;

    float _level = ZERO_DB_NORMALIZED_VALUE;
    float _pan = 0.5f;
    bool _mute = false;
    bool _solo = false;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Fader, _level, _pan, _mute, _solo);

private:
    float dbToNormalizedValue(float db);
    float dbToLinear(float db);
    float linearToGainRatio(float linearValue);
    float gainRatioToLinear(float gainRatio);
    float normalizedValueToDb(float linear);

    float _peakValueLeft = 0.0f;
    float _peakValueRight = 0.0f;
    float _peakValueHoldLeft = 0.0f;
    float _peakValueHoldRight = 0.0f;
    int _peakSampleElapsedLeft = 0;
    int _peakSampleElapsedRight = 0;
};

