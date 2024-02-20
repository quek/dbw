#include "DeleteModule.h"
#include <mutex>
#include "../AudioEngine.h"
#include "../Composer.h"
#include "../Module.h"
#include "../Track.h"

command::DeleteModule::DeleteModule(Module* module) : _module(module) {
}
void command::DeleteModule::execute(Composer* composer) {
    _module->closeGui();
    _module->stop();
    auto modules = &_module->_track->_modules;
    std::lock_guard<std::recursive_mutex> lock(composer->_audioEngine->_mtx);
    auto it = std::find_if(modules->begin(), modules->end(), [this](const std::unique_ptr<Module>& ptr) {
        return ptr.get() == _module;
    });

    // 見つかった場合
    if (it != modules->end()) {
        _moduleUniquePtr = std::move(*it);
        _index = std::distance(modules->begin(), it);
        modules->erase(it);
    }

    composer->computeProcessOrder();
}

void command::DeleteModule::undo(Composer* composer) {
    std::lock_guard<std::recursive_mutex> lock(composer->_audioEngine->_mtx);
    _module->_track->_modules.insert(_module->_track->_modules.begin() + _index, std::move(_moduleUniquePtr));

    _module->start();
    composer->computeProcessOrder();
    _module->openGui();
}

