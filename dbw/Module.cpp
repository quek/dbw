#include "Module.h"
#include <mutex>
#include "imgui.h"
#include "AudioEngine.h"
#include "Composer.h"
#include "Command.h"

class DeleteModuleCommand : public Command {
public:
    DeleteModuleCommand(Module* module) : _module(module) {
    }

    void execute(Composer* composer) override {
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

    void undo(Composer* composer) override {
        std::lock_guard<std::mutex> lock(composer->_audioEngine->mtx);
        _module->_track->_modules.insert(_module->_track->_modules.begin() + _index, std::move(_moduleUniquePtr));

        _module->start();
        _module->openGui();
    }

    Module* _module;
    int _index = 0;
    std::unique_ptr<Module> _moduleUniquePtr;
};

void Module::render() {
    if (ImGui::Button(_name.c_str())) {
        if (_didOpenGui) {
            closeGui();
        } else {
            openGui();
        }
    }
    if (ImGui::BeginPopupContextItem("MyButtonContextMenu")) {
        if (ImGui::MenuItem("Delete")) {
            _track->_composer->_commandManager.executeCommand(new DeleteModuleCommand(this));
        }
        ImGui::EndPopup();
    }
}

void Module::process(ProcessBuffer* /* in */, unsigned long framesPerBuffer, int64_t /* steadyTime */) {
    _processBuffer.ensure(framesPerBuffer);
}

