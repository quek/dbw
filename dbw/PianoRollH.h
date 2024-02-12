#pragma once
#include <memory>
#include <string>
#include <imgui.h>
#include <set>
#include "GridMixin.h"
#include "ZoomMixin.h"

class Clip;
class Composer;
class Grid;
class Note;
struct Bounds;

class PianoRollH : public GridMixin, public ZoomMixin {
public:
    PianoRollH(Composer* composer);
    virtual ~PianoRollH() = default;
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

    Bounds boundsOfNote(Note* note) const;
    void handleCanvas();
    Note* noteFromMousePos();
    double noteTimeFromMouserPos(float offset = 0.0f);
    int16_t noteKeyFromMouserPos();
    ImVec2 toCanvasPos(ImVec2& pos) const;
    double toSnapFloor(const double time);
    double toSnapRound(const double time);
    bool isInCanvas(ImVec2& pos);

    Composer* _composer;
    bool _show = false;
    Clip* _clip = nullptr;
    std::string _scrollHereYKey = "";

    enum NoteClickedPart {
        Left,
        Middle,
        Right
    };
    struct State {
        Note* _clickedNote = nullptr;
        std::set<Note*> _selectedNotes;
        Note* _draggingNote = nullptr;
        bool _unselectClickedNoteIfMouserReleased = false;
        NoteClickedPart _noteClickedPart = Middle;
        float _noteClickedOffset = 0.0f;
        bool _rangeSelecting = false;

        bool _consumedDoubleClick = false;
        bool _consumedClicked = false;
        void reset();
    };
    State _state;
};

extern std::unique_ptr<PianoRollH> gPianoRoll;
