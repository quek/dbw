#pragma once
#include <memory>

class Clip;

class PianoRoll {
public:
    void render();
    void edit(Clip* clip);

private:
    bool _show = false;
    Clip* _clip;
};

extern std::unique_ptr<PianoRoll> gPianoRoll;
