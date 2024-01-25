#pragma once
#include <functional>
#include <stack>
#include <memory>

class Composer;

class Command
{
public:
    virtual ~Command() = default;
    virtual void execute(Composer* composer) = 0;
    virtual void undo(Composer* composer) = 0;
private:
    Composer* _composer;
};

class CommandManager {

public:
    CommandManager(Composer* composer);
    void executeCommand(Command* command) {
        command->execute(_composer);
        std::shared_ptr<Command> p(command);
        _undoStack.push(p);
        _redoStack = std::stack<std::shared_ptr<Command>>();
    }

    void undo() {
        if (!_undoStack.empty()) {
            auto& command = _undoStack.top();
            command->undo(_composer);
            _redoStack.push(command);
            _undoStack.pop();
        }
    }

    void redo() {
        if (!_redoStack.empty()) {
            auto& command = _redoStack.top();
            command->execute(_composer);
            _undoStack.push(command);
            _redoStack.pop();
        }
    }
private:
    Composer* _composer;
    std::stack<std::shared_ptr<Command>> _undoStack;
    std::stack<std::shared_ptr<Command>> _redoStack;
};
