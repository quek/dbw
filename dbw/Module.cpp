#include "Module.h"
#include <mutex>
#include "imgui.h"
#include "AudioEngine.h"
#include "Composer.h"
#include "Command.h"

class DeleteModuleCommand : public Command {
public:
    DeleteModuleCommand(Module* module) : _module(module) {}
    void execute(Composer* composer) override {
        _module->closeGui();
        _module->stop();
        std::lock_guard<std::mutex> lock(composer->_audioEngine->mtx);
        auto it = std::find_if(_track->_modules.begin(), _track->_modules.end(), [this](const std::unique_ptr<Module>& ptr) {
            return ptr.get() == _module;
            });

        // 見つかった場合
        if (it != _track->_modules.end()) {
            // 所有権を移動
            _moduleUniquePtr = std::move(*it);

            // ベクターから要素を削除
            _track->_modules.erase(it);
        }

    }
    void undo(Composer* composer) override {
        std::lock_guard<std::mutex> lock(composer->_audioEngine->mtx);
        // TODO _module = std::move(_track->_modules.back());
        _track->_modules.pop_back();

        _track->_modules.back()->start();
        _track->_modules.back()->openGui();
    }
    Track* _track;
    Module* _module;
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
            // アクション1が選択された時の処理
        }
        ImGui::EndPopup();
    }
}

void Module::process(ProcessBuffer* /* in */, unsigned long framesPerBuffer, int64_t /* steadyTime */) {
    _processBuffer.ensure(framesPerBuffer);
}

