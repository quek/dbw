#include "AudioEngine.h"
#include <mutex>
#include <ranges>
#include "Command.h"
#include "Composer.h"
#include "Track.h"

Command::Command(bool undoable) : _undoable(undoable) {
}

GroupCommand::GroupCommand(std::vector<Command*> commands, bool undoable) : Command(undoable) {
    for (auto command : commands) {
        _commands.emplace_back(command);
    }
}

void GroupCommand::execute(Composer* composer) {
    for (auto& command : _commands) {
        command->execute(composer);
    }
}

void GroupCommand::undo(Composer* composer) {
    for (auto& command : _commands | std::views::reverse) {
        command->undo(composer);
    }
}

CommandManager::CommandManager(Composer* composer) : _composer(composer) {
}

// iterator でまわっているときに要素の削除をすると落ちるので
// いいたんキューにためて安全な場所で実行する
void CommandManager::executeCommand(Command* command) {
    std::shared_ptr<Command> p(command);
    _queue.push(p);
}

void CommandManager::executeCommand(std::vector<Command*> commands, bool undoable) {
    Command* command = new GroupCommand(commands, undoable);
    executeCommand(command);
}

void CommandManager::run() {
    while (!_queue.empty()) {
        std::shared_ptr<Command> command = _queue.front();
        _queue.pop();
        command->execute(_composer);
        if (command->_undoable) {
            _undoStack.push(command);
        }
        _redoStack = std::stack<std::shared_ptr<Command>>();
    }
}

void CommandManager::undo() {
    if (!_undoStack.empty()) {
        auto& command = _undoStack.top();
        command->undo(_composer);
        _redoStack.push(command);
        _undoStack.pop();
    }
}

void CommandManager::redo() {
    if (!_redoStack.empty()) {
        auto& command = _redoStack.top();
        command->execute(_composer);
        _undoStack.push(command);
        _redoStack.pop();
    }
}

void CommandManager::clear() {
    std::queue<std::shared_ptr<Command>> queue;
    _queue.swap(queue);
    std::stack<std::shared_ptr<Command>> undoStack;
    _undoStack.swap(undoStack);
    std::stack<std::shared_ptr<Command>> redoStack;
    _redoStack.swap(redoStack);
}

