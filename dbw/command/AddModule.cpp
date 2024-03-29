#include "AddModule.h"
#include <mutex>
#include <ranges>
#include "../App.h"
#include "../Composer.h"
#include "../Track.h"

command::AddModule::AddModule(NekoId trackRef, const char* type, const std::string& id, bool openGui) :
    _trackRef(trackRef), _type(type), _id(id), _openGui(openGui) {
}

void command::AddModule::execute(Composer* composer) {
    auto module = exec(composer);
    if (!module) {
        return;
    }
    if (_openGui) {
        module->openGui();
    }
    composer->computeProcessOrder();
}

void command::AddModule::redo(Composer* composer) {
    exec(composer);
    composer->computeProcessOrder();
}

void command::AddModule::undo(Composer* composer) {
    auto track = Neko::findByNekoId<Track>(_trackRef);
    auto it = std::ranges::find_if(track->_modules, [this](const auto& x) { return x->getNekoId() == _moduleId; });
    if (it != track->_modules.end()) {
        std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
        (*it)->stop();
        track->_modules.erase(it);
        track->getComposer()->computeProcessOrder();
    }
}

Module* command::AddModule::exec(Composer* composer) {
    auto track = Neko::findByNekoId<Track>(_trackRef);
    if (!track) {
        return nullptr;
    }
    auto module = Module::create(_type, _id);
    _moduleId = module->getNekoId();

    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
    track->addModule(module);
    return module;
}

