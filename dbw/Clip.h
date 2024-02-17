#pragma once
#include <memory>
#include "Sequence.h"
#include "Thing.h"
#include "XMLMixin.h"

class PianoRollWindow;

class Clip : public Nameable, public Thing, public XMLMixin {
public:
    Clip(const Clip& other) = default;
    Clip(double time = 0.0, double duration = 16.0);
    Clip(double time, double duration, std::shared_ptr<Sequence> sequence);
    Clip(std::shared_ptr<Sequence> sequence);
    virtual ~Clip() = default;

    std::string name() const;
    void renderInScene(PianoRollWindow* pianoRollWindow);
    tinyxml2::XMLElement* toXml(tinyxml2::XMLDocument* doc) override;
    static std::unique_ptr<Clip> fromXml(tinyxml2::XMLElement* element);


    std::shared_ptr<Sequence> _sequence;

    bool _selected = false;
};
