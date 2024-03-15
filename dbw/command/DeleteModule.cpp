#include "DeleteModule.h"
#include <mutex>
#include "../App.h"
#include "../Composer.h"
#include "../Module.h"
#include "../SerializeContext.h"
#include "../Track.h"

command::DeleteModule::DeleteModule(Module* module) : _moduleId(module->getNekoId()), _trackId(module->_track->getNekoId()) {
}

void command::DeleteModule::execute(Composer* composer) {
    Module* module = Neko::findByNekoId<Module>(_moduleId);
    if (module == nullptr) {
        return;
    }
    module->closeGui();
    module->stop();
    SerializeContext context;
    _module = module->toJson(context);

    auto modules = &module->_track->_modules;

    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);

    auto it = std::find_if(modules->begin(), modules->end(), [module](const std::unique_ptr<Module>& ptr) {
        return ptr.get() == module;
    });
    if (it != modules->end()) {
        _index = std::distance(modules->begin(), it);
        modules->erase(it);
    }

    composer->computeProcessOrder();

}

void command::DeleteModule::undo(Composer* composer) {
    SerializeContext context;
    Module* module = Module::fromJson(_module, context);
    Track* track = Neko::findByNekoId<Track>(_trackId);
    module->_track = track;

    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
    track->_modules.emplace(track->_modules.begin() + _index, module);
    module->start();
    composer->computeProcessOrder();
}
