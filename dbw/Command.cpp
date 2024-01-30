#include "AudioEngine.h"
#include <mutex>
#include "Command.h"
#include "Composer.h"
#include "Line.h"
#include "Track.h"

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

AddColumnCommand::AddColumnCommand(Track* track) : _track(track) {
    for (auto line = _track->_lines.begin(); line != _track->_lines.end(); ++line) {
        _columns.push_back(std::make_unique<Column>((*line).get()));
    }
}

void AddColumnCommand::execute(Composer* composer) {
    std::lock_guard<std::mutex> lock(composer->_audioEngine->mtx);
    for (auto [line, column] : std::views::zip(_track->_lines, _columns)) {
        line->_columns.push_back(std::move(column));
    }
    _track->_ncolumns++;
    _track->_lastKeys.push_back(0);
}

void AddColumnCommand::undo(Composer* composer) {
    std::lock_guard<std::mutex> lock(composer->_audioEngine->mtx);
    for (auto [line, column] : std::views::zip(_track->_lines, _columns)) {
        column = std::move(line->_columns.back());
        line->_columns.pop_back();
    }
    _track->_ncolumns--;
    _track->_lastKeys.pop_back();
}

DeleteColumnCommand::DeleteColumnCommand(Track* track) : _track(track) {
    for (auto i = 0; i < _track->_composer->_maxLine; ++i) {
        _columns.push_back(std::make_unique<Column>(nullptr));
    }
}

void DeleteColumnCommand::execute(Composer* composer) {
    std::lock_guard<std::mutex> lock(composer->_audioEngine->mtx);
    for (auto [line, column] : std::views::zip(_track->_lines, _columns)) {
        column = std::move(line->_columns.back());
        line->_columns.pop_back();
    }
    _track->_ncolumns--;
    _track->_lastKeys.pop_back();
}

void DeleteColumnCommand::undo(Composer* composer) {
    std::lock_guard<std::mutex> lock(composer->_audioEngine->mtx);
    for (auto [line, column] : std::views::zip(_track->_lines, _columns)) {
        line->_columns.push_back(std::move(column));
    }
    _columns.clear();
    _track->_ncolumns++;
    _track->_lastKeys.push_back(0);
}

DeleteModuleCommand::DeleteModuleCommand(Module* module) : _module(module) {
}

void DeleteModuleCommand::execute(Composer* composer) {
    _module->closeGui();
    _module->stop();
    auto modules = &_module->_track->_modules;
    std::lock_guard<std::mutex> lock(composer->_audioEngine->mtx);
    auto it = std::find_if(modules->begin(), modules->end(), [this](const std::unique_ptr<Module>& ptr) {
        return ptr.get() == _module;
    });

    // 見つかった場合
    if (it != modules->end()) {
        _moduleUniquePtr = std::move(*it);
        _index = std::distance(modules->begin(), it);
        modules->erase(it);
    }
}

void DeleteModuleCommand::undo(Composer* composer) {
    std::lock_guard<std::mutex> lock(composer->_audioEngine->mtx);
    _module->_track->_modules.insert(_module->_track->_modules.begin() + _index, std::move(_moduleUniquePtr));

    _module->start();
    _module->openGui();
}
