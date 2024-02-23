#pragma once
#include "BuiltinModule.h"

class GainModule : public BuiltinModule {
public:
    GainModule(const nlohmann::json& json);
    GainModule(std::string name = "Gain", Track* track = nullptr);
    virtual ~GainModule() = default;
    virtual nlohmann::json toJson() override;
    bool process(ProcessBuffer* buffer, int64_t steadyTime) override;
    void renderContent() override;
private:
    float _gain;
};

