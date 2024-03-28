#pragma once
#include <string>
#include "TimelineCanvasMixin.h"

class NoteClip;
class Command;
class Composer;
class Note;

class PianoRollWindow : public TimelineCanvasMixin<Note, int16_t>
{
public:
    PianoRollWindow(Composer* composer);
    virtual ~PianoRollWindow() = default;
    void render() override;
    void edit(NoteClip* clip);

    void handleDoubleClick(Note* thing) override;
    Note* handleDoubleClick(double time, int16_t* lane) override;
    void handleMove(double oldTime, double newTime, int16_t* oldLane, int16_t* newLane) override;
    void handleClickTimeline(double time, bool ctrl, bool alt) override;
    std::pair<std::set<Note*>, Command*> copyThings(std::set<Note*> srcs, bool redoable) override;
    Command* deleteThings(std::set<Note*>& notes, bool undoable) override;
    Command* duplicateThings(std::set<Note*>& things, bool undoable) override;
    Command* splitThings(std::set<Note*>& things, double time) override;

    void prepareAllThings() override;

    float offsetTop() const override;
    float offsetLeft() const override;
    float offsetStart() const override;

    int16_t* laneFromPos(ImVec2& pos) override;
    float xFromThing(Note* thing) override;
    float laneToScreenX(int16_t* lane);
    float getLaneWidth(Note* thing) override;

    NoteClip* _clip = nullptr;
    std::string _scrollHereXKey = "";

protected:
    void handleShortcut() override;
    void renderPlayhead() override;
    void renderHeader() override;
    void renderSequenceEnd();
    void renderTimeline() override;
    std::string windowName() override;
    std::string canvasName() override;

private:
    bool _fitContentRequired = false;
    bool fitContent();
};
