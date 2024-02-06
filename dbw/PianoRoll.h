#pragma once
#include <memory>
#include <string>
#include <imgui.h>
#include <set>

class Clip;
class Grid;
class Note;

class PianoRoll {
public:
    PianoRoll();
    virtual ~PianoRoll() = default;
    void render();
    void edit(Clip* clip);

private:
    int maxBar();
    void renderBackgroud() const;
    void renderGrid();
    void renderGridBeat16th(ImDrawList* drawList, float beatX, float y1, float y2) const;
    void renderKeyboard();
    void renderNotes();
    void renderTimeline();

    void handleCanvas();
    Note* noteFromMousePos();
    double noteTimeFromMouserPos();
    int16_t noteKeyFromMouserPos();
    ImVec2 toCanvasPos(ImVec2& pos) const;
    double toSnap(const double time);
    bool isInCanvas(ImVec2& pos);

    bool _show = false;
    Clip* _clip = nullptr;
    Grid* _grid;
    bool _snap = true;
    float _zoomX = 4.0f;
    float _zoomY = 0.5f;
    std::string _scrollHereYKey = "";

    struct State {
        Note* _clickedNote = nullptr;
        std::set<Note*> _selectedNotes;
        Note* _draggingNote = nullptr;
        bool _unselectClickedNoteIfMouserReleased = false;

        bool _consumedDoubleClick = false;
        bool _consumedClicked = false;
        void reset();

        int foo;
        int bar;
        int baz;
    };
    State _state;
};

extern std::unique_ptr<PianoRoll> gPianoRoll;
