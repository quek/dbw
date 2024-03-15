#pragma once
#include "BuiltinModule.h"

class GainModule : public BuiltinModule {
public:
    GainModule(const nlohmann::json& json, SerializeContext& context);
    GainModule(std::string name = "Gain", Track* track = nullptr);
    virtual ~GainModule() = default;
    virtual nlohmann::json toJson(SerializeContext& context) override;
    bool process(ProcessBuffer* buffer, int64_t steadyTime) override;
    void renderContent() override;
private:
    float _gain;
};

