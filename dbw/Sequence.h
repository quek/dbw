#pragma once
#include <memory>
#include <vector>
#include "Command.h"
#include "Nameable.h"
#include "Note.h"
#include "Neko.h"

class Composer;

class Sequence : public Nameable {
public:
    static std::shared_ptr<Sequence>create(double duration = 16.0, uint64_t id = 0);
    virtual ~Sequence();
    tinyxml2::XMLElement* toXml(tinyxml2::XMLDocument* doc) override;
    static std::shared_ptr<Sequence> fromXml(tinyxml2::XMLElement* element);

    std::vector<std::unique_ptr<Note>> _notes;
    double _duration;

    static int _no;

private:
    Sequence(double duration);
};

class DeleteNoteCommand : public Command {
public:
    DeleteNoteCommand(Sequence* sequence, Note* note, bool undoable = true);
    void execute(Composer* composer);
    void undo(Composer* composer);

    Sequence* _sequence;
    Note* _note;
    std::unique_ptr<Note> _noteForUndo;
};
