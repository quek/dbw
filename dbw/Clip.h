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
    virtual ~Clip() = default;

    virtual std::string name() const;
    std::shared_ptr<Sequence>& getSequence() { return _sequence; }

    virtual nlohmann::json toJson() override;

    double _time;
    double _duration;

    bool _selected = false;
    std::shared_ptr<Sequence> _sequence;
};
