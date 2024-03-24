#pragma once
#include "BuiltinModule.h"

class Fader : public BuiltinModule {
public:
    const float ZERO_DB_METER = 0.7f;
    const float ZERO_DB_SLIDER = 0.7f;

    Fader(const nlohmann::json& json, SerializeContext& context);
    Fader(std::string name, Track* track);
    virtual ~Fader() = default;
    bool process(ProcessBuffer* buffer, int64_t steadyTime) override;
    void render(float width = 0.0f, float height = 0.0f) override;

    virtual nlohmann::json toJson(SerializeContext& context) override;

    float _level = ZERO_DB_METER;
    float _pan = 0.5f;
    bool _mute = false;
    bool _solo = false;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Fader, _level, _pan, _mute, _solo);

private:
    float dbToNormalizedValue(float db);
    float dbToMeter(float db);
    float gainRatioToLevel(float gainRatio);
    float levelToGainRatio(float level);
    float normalizedValueToDb(float linear);
    float normalizedValueToMeter(float value);

    float _peakValueLeft = 0.0f;
    float _peakValueRight = 0.0f;
    float _peakValueHoldLeft = 0.0f;
    float _peakValueHoldRight = 0.0f;
    int _peakSampleElapsedLeft = 0;
    int _peakSampleElapsedRight = 0;
};

