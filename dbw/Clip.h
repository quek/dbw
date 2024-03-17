#pragma once
#include <memory>
#include "Sequence.h"
#include "Neko.h"

class Lane;
class PianoRollWindow;

class Clip : public Nameable {
public:
    static Clip* create(const nlohmann::json& json, SerializeContext& context);
    Clip(const nlohmann::json& json, SerializeContext& context);
    Clip(double time = 0.0, double duration = 16.0);
    virtual ~Clip() = default;
    virtual Clip* clone() = 0;
    virtual void edit(Composer* composer, Lane* lane) = 0;
    virtual std::string name() const;
    std::shared_ptr<Sequence>& getSequence() { return _sequence; }
    virtual void prepareProcessBuffer(Lane* lane, double begin, double end, double loopBegin, double loopEnd, double oneBeatSec) ;
    virtual void render(const ImVec2& pos1, const ImVec2& pos2, const ImVec2& canvasPos1, const ImVec2& canvasPos2, const bool selected);
    virtual void renderInScene(PianoRollWindow* pianoRollWindow) = 0;

    virtual nlohmann::json toJson(SerializeContext& context) override;

    double _time;
    double _duration;

    bool _selected = false;
    std::shared_ptr<Sequence> _sequence;
};
