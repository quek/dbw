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
    inline static const char* TYPE = "sequence";
    static std::shared_ptr<Sequence>create(double duration = 16.0, NekoId id = 0);
    static std::shared_ptr<Sequence>create(const nlohmann::json& json);
    virtual ~Sequence();

    virtual nlohmann::json toJson() override;

    std::vector<std::unique_ptr<Note>> _notes;
    double _duration;

    static int _no;

private:
    Sequence() = default;
    Sequence(const nlohmann::json& josn);
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
