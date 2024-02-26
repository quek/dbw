#include "AddModule.h"
#include <mutex>
#include <ranges>
#include "../App.h"
#include "../Composer.h"
#include "../Track.h"

command::AddModule::AddModule(uint64_t trackRef, const char* type, const std::string& id, bool openGui) :
    _trackRef(trackRef), _type(type), _id(id), _openGui(openGui) {
}

void command::AddModule::execute(Composer* composer) {
    auto module = exec(composer);
    if (_openGui) {
        module->openGui();
    }
}

void command::AddModule::redo(Composer* composer) {
    exec(composer);
}

void command::AddModule::undo(Composer* composer) {
    auto track = Neko::findByNekoId<Track>(_trackRef);
    auto it = std::ranges::find_if(track->_modules, [this](const auto& x) { return x->nekoId() == _moduleId; });
    if (it != track->_modules.end()) {
        std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
        (*it)->stop();
        track->_modules.erase(it);
        track->getComposer()->computeProcessOrder();
    }
}

Module* command::AddModule::exec(Composer* composer) {
    auto module = Module::create(_type, _id);
    _moduleId = module->nekoId();
    auto track = Neko::findByNekoId<Track>(_trackRef);

    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
    track->addModule(module);
    return module;
}

