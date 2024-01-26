#pragma once
#include <functional>
#include <stack>
#include <memory>
#include <queue>

class Composer;

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
private:
    Composer* _composer;
    std::queue<std::shared_ptr<Command>> _queue;
    std::stack<std::shared_ptr<Command>> _undoStack;
    std::stack<std::shared_ptr<Command>> _redoStack;
};
