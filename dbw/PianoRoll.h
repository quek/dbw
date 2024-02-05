#pragma once
#include <memory>
#include <imgui.h>

class Clip;
class Grid;

class PianoRoll {
public:
    PianoRoll();
    virtual ~PianoRoll() = default;
    void render();
    void edit(Clip* clip);

private:
    int maxBar();
    void renderBackgroud();
    void renderGrid();
    void renderGridBeat16th(ImDrawList* drawList, float beatX, float y1, float y2);
    void renderKeyboard();
    void renderTimeline();

    void handleCanvas();

    bool _show = false;
    Clip* _clip;
    Grid* _grid;
    bool _snap = true;
    float _zoomX = 1.0f;
    float _zoomY = 0.5f;

    struct State {
        int foo = 0;
    };
    State _state;
};

extern std::unique_ptr<PianoRoll> gPianoRoll;
