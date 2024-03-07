#pragma once
#include <memory>
#include "Sequence.h"
#include "Neko.h"

class Lane;
class PianoRollWindow;

class Clip : public Nameable {
public:
    static Clip* create(const nlohmann::json& json);
    Clip(const nlohmann::json& json);
    Clip(double time = 0.0, double duration = 16.0);
    virtual ~Clip() = default;
    virtual Clip* clone() = 0;
    virtual void edit(Composer* composer, Lane* lane) = 0;
    virtual std::string name() const;
    std::shared_ptr<Sequence>& getSequence() { return _sequence; }
    virtual void renderInScene(PianoRollWindow* pianoRollWindow) = 0;

    virtual nlohmann::json toJson() override;

    double _time;
    double _duration;

    bool _selected = false;
    std::shared_ptr<Sequence> _sequence;
};
