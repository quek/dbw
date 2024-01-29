#include "AudioEngine.h"
#include "Command.h"
#include "Composer.h"
#include "Module.h"
#include "Track.h"
#include <mutex>

CommandManager::CommandManager(Composer* composer) : _composer(composer) {
}

// iterator でまわっているときに要素の削除をすると落ちるので
// いいたんキューにためて安全な場所で実行する
void CommandManager::executeCommand(Command* command) {
    std::shared_ptr<Command> p(command);
    _queue.push(p);
}

void CommandManager::run() {
    while (!_queue.empty()) {
        std::shared_ptr<Command> command = _queue.front();
        _queue.pop();
        command->execute(_composer);
        _undoStack.push(command);
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

void AddModuleCommand::execute(Composer* composer) {
    std::lock_guard<std::mutex> lock(composer->_audioEngine->mtx);
    _track->_modules.push_back(std::move(_module));
    _track->_modules.back()->start();
    _track->_modules.back()->openGui();
}

void AddModuleCommand::undo(Composer* composer) {
    std::lock_guard<std::mutex> lock(composer->_audioEngine->mtx);
    _module = std::move(_track->_modules.back());
    _track->_modules.pop_back();
    _module->closeGui();
    _module->stop();
}
