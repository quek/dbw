#include "AddModule.h"
#include <mutex>
#include "../AudioEngine.h"
#include "../Composer.h"
#include "../Track.h"

void command::AddModule::execute(Composer* composer) {
    std::lock_guard<std::recursive_mutex> lock(composer->_audioEngine->_mtx);
    auto module = _module.get();
    _track->_modules.emplace_back(std::move(_module));
    module->start();
    composer->computeProcessOrder();
    module->openGui();
}

void command::AddModule::undo(Composer* composer) {
    std::lock_guard<std::recursive_mutex> lock(composer->_audioEngine->_mtx);
    _module = std::move(_track->_modules.back());
    _track->_modules.pop_back();
    _module->closeGui();
    _module->stop();
    composer->computeProcessOrder();
}

command::AddModule2::AddModule2(uint64_t trackRef, std::string& type, std::string& id) :
    _trackRef(trackRef), _type(type), _id(id) {
}

void command::AddModule2::execute(Composer* composer) {
    auto track = Neko::findByNekoId<Track>(_trackRef);
    auto module = Module::create(_type, _id);
    track->_modules.emplace_back(module);
}

void command::AddModule2::undo(Composer* composer) {
}
