#pragma once
#include <functional>
#include <stack>
#include <memory>
#include <queue>
#include "Column.h"
#include "Module.h"

class Composer;
class Track;

class Command {
public:
    virtual ~Command() = default;
    virtual void execute(Composer* composer) = 0;
    virtual void undo(Composer* composer) = 0;
};

class CommandManager {

public:
    CommandManager(Composer* composer);
    void executeCommand(Command* command);
    void run();
    void undo();
    void redo();
    void clear();
private:
    Composer* _composer;
    std::queue<std::shared_ptr<Command>> _queue;
    std::stack<std::shared_ptr<Command>> _undoStack;
    std::stack<std::shared_ptr<Command>> _redoStack;
};

template <typename T, typename P>
class EditProperty : public Command {
public:
    EditProperty(T* column, P T::* x, P T::* lastX, P value, P lastValue)
        : _column(column), _x(x), _lastX(lastX), _value(value), _lastValue(lastValue) {
    }

    void execute(Composer* /*composer*/) override {
        _column->*_x = _value;
        _column->*_lastX = _value;
    }

    void undo(Composer* /*composer*/) override {
        _column->*_x = _lastValue;
        _column->*_lastX = _lastValue;
    }

private:
    T* _column;
    P T::* _x;
    P T::* _lastX;
    P _value;
    P _lastValue;
};

class AddModuleCommand : public Command {
public:
    AddModuleCommand(Track* track, Module* module) : _track(track), _module(module) {}
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;
    Track* _track;
    std::unique_ptr<Module> _module;
};

class AddColumnCommand : public Command {
public:
    AddColumnCommand(Track* track);
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;
    Track* _track;
    std::vector<std::unique_ptr<Column>> _columns;
};

class DeleteColumnCommand : public Command {
public:
    DeleteColumnCommand(Track* track);
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;
    Track* _track;
    std::vector<std::unique_ptr<Column>> _columns;
};

