#include "App.h"
#include "Composer.h"
#include "Command.h"

App::App() :
    _audioEngine(std::make_unique<AudioEngine>(this)) {
    addComposer(new Composer());
}

void App::addComposer(Composer* composer) {
    composer->_app = this;
    _composers.emplace_back(composer);
}

void App::deleteComposer(Composer* composer) {
    auto it = std::ranges::find_if(_composers, [composer](const auto& x) { return x.get() == composer; });
    if (it != _composers.end()) {
        _composers.erase(it);
    }
}

void App::runCommand() {
    _requestAddComposers.clear();
    _requestDeleteComposers.clear();

    for (auto& composer : _composers) {
        composer->_commandManager.run();
    }

    if (_requestAddComposers.empty() && _requestDeleteComposers.empty()) {
        return;
    }

    _audioEngine->stop();
    for (auto* composer : _requestDeleteComposers) {
        deleteComposer(composer);
    }
    for (auto* composer : _requestAddComposers) {
        addComposer(composer);
        composer->computeProcessOrder();
    }
    _audioEngine->start();
}

void App::requestAddComposer(Composer* composer) {
    _requestAddComposers.push_back(composer);
}

void App::requestDeleteComposer(Composer* composer) {
    _requestDeleteComposers.push_back(composer);
}
