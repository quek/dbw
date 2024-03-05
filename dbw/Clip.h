#pragma once
#include <memory>
#include "Sequence.h"
#include "Neko.h"

class PianoRollWindow;

class Clip : public Nameable {
public:
    inline static const char* TYPE = "clip";
    Clip(const nlohmann::json& json);
    Clip(double time = 0.0, double duration = 16.0);
    Clip(double time, double duration, std::shared_ptr<Sequence> sequence);
    Clip(std::shared_ptr<Sequence> sequence);
    virtual ~Clip() = default;

    std::string name() const;
    void renderInScene(PianoRollWindow* pianoRollWindow);

    virtual nlohmann::json toJson() override;

    double _time;
    double _duration;
    std::shared_ptr<Sequence> _sequence;

    bool _selected = false;
};
