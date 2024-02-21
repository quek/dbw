#include "AddModule.h"
#include <mutex>
#include <ranges>
#include "../AudioEngine.h"
#include "../Composer.h"
#include "../Track.h"

command::AddModule::AddModule(uint64_t trackRef, const char* type, const std::string& id) :
    _trackRef(trackRef), _type(type), _id(id) {
}

void command::AddModule::execute(Composer* composer) {
    exec(composer)->openGui();
}

void command::AddModule::redo(Composer* composer) {
    exec(composer);
}

void command::AddModule::undo(Composer* composer) {
    auto track = Neko::findByNekoId<Track>(_trackRef);
    auto it = std::ranges::find_if(track->_modules, [this](const auto& x) { return x->xmlId() == _moduleId; });
    if (it != track->_modules.end()) {
        std::lock_guard<std::recursive_mutex> lock(composer->_audioEngine->_mtx);
        (*it)->stop();
        track->_modules.erase(it);
        track->_composer->computeProcessOrder();
    }
}

Module* command::AddModule::exec(Composer* composer) {
    auto module = Module::create(_type, _id);
    _moduleId = module->xmlId();
    auto track = Neko::findByNekoId<Track>(_trackRef);
    module->_track = track;

    std::lock_guard<std::recursive_mutex> lock(composer->_audioEngine->_mtx);
    track->_modules.emplace_back(module);
    module->start();
    track->_composer->computeProcessOrder();
    return module;
}

