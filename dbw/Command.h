#pragma once
#include <functional>
#include <stack>
#include <memory>
#include <queue>
#include <vector>
#include "Module.h"

class Composer;
class Track;

class Command {
public:
    Command(bool undoable = true);
    virtual ~Command() = default;
    virtual void execute(Composer* composer) = 0;
    virtual void undo(Composer* composer) = 0;
    virtual void redo(Composer* composer);
    bool _undoable = true;
};

class GroupCommand : public Command {

public:
    GroupCommand(std::vector<Command*> commands, bool undoable = true);
    virtual ~GroupCommand() = default;
    virtual void execute(Composer* composer) override;
    virtual void undo(Composer* composer) override;
    std::vector<std::unique_ptr<Command>> _commands;
};

class CommandManager {
public:
    CommandManager(Composer* composer);
    void executeCommand(Command* command);
    void executeCommand(std::vector<Command*> commands, bool undoable);
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
