#pragma once
#include <functional>
#include <stack>
#include <memory>

class Composer;

class Command
{
public:
    Command(std::function<void(Composer*)> execute, std::function<void(Composer*)> undo)
        : _execute(execute), _undo(undo) {}
    virtual ~Command() = default;

    std::function<void(Composer*)> _execute;
    std::function<void(Composer*)> _undo;
};

class CommandManager {

public:
    CommandManager(Composer* composer);
    void executeCommand(std::shared_ptr<Command> command) {
        command->_execute(_composer);
        _undoStack.push(command);
        _redoStack = std::stack<std::shared_ptr<Command>>();
    }

    void undo() {
        if (!_undoStack.empty()) {
            auto& command = _undoStack.top();
            command->_undo(_composer);
            _redoStack.push(command);
            _undoStack.pop();
        }
    }

    void redo() {
        if (!_redoStack.empty()) {
            auto& command = _redoStack.top();
            command->_execute(_composer);
            _undoStack.push(command);
            _redoStack.pop();
        }
    }
private:
    Composer* _composer;
    std::stack<std::shared_ptr<Command>> _undoStack;
    std::stack<std::shared_ptr<Command>> _redoStack;
};
