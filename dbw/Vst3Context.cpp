#include "Vst3Context.h"
#include <public.sdk/source/vst/utility/stringconvert.h>
#include <public.sdk/source/vst/hosting/hostclasses.h>
#include "PluginEditorWindow.h"
#include "Vst3Module.h"
#include "Error.h"

using namespace Steinberg;

Vst3Context::Vst3Context(Vst3Module* module) : _module(module) {
}

Vst3Context::~Vst3Context() {
}

tresult PLUGIN_API Vst3Context::getName(Vst::String128 name) {
    return VST3::StringConvert::convert("DBW", name) ? kResultTrue :
        kInternalError;
}

tresult PLUGIN_API Vst3Context::createInstance(TUID cid, TUID _iid, void** obj) {
    if (FUnknownPrivate::iidEqual(cid, Vst::IMessage::iid) &&
        FUnknownPrivate::iidEqual(_iid, Vst::IMessage::iid)) {
        *obj = new Vst::HostMessage;
        return kResultTrue;
    }
    if (FUnknownPrivate::iidEqual(cid, Vst::IAttributeList::iid) &&
        FUnknownPrivate::iidEqual(_iid, Vst::IAttributeList::iid)) {
        if (auto al = Vst::HostAttributeList::make()) {
            *obj = al.take();
            return kResultTrue;
        }
        return kOutOfMemory;
    }
    *obj = nullptr;
    return kResultFalse;
}

tresult PLUGIN_API Vst3Context::beginEdit(Vst::ParamID id) {
    _module->beginEdit(id);
    return kResultOk;
}

tresult PLUGIN_API Vst3Context::performEdit(Vst::ParamID id, Vst::ParamValue valueNormalized) {
    _module->performEdit(id, valueNormalized);
    return kResultOk;
}

tresult PLUGIN_API Vst3Context::endEdit(Vst::ParamID id) {
    _module->endEdit(id);
    return kResultOk;
}

tresult PLUGIN_API Vst3Context::restartComponent(int32 flags) {
    if (_module->_processor == nullptr) {
        return kResultOk;
    }
    // TODO
    Error("restartComponent {}", flags);
    if (_module->_track) {
        _module->stop();
        _module->start();
        // TODO 他にもフラグ見て色々する
        if ((flags & Vst::RestartFlags::kParamValuesChanged) != 0) {
            _module->prepareParameterValue();
        } else if ((flags & Vst::RestartFlags::kParamTitlesChanged) != 0) {
            _module->prepareParameterInfo();
        }
    }
    return kResultOk;
}

tresult PLUGIN_API Vst3Context::resizeView(IPlugView* /*view*/, ViewRect* newSize) {
    if (_module->_editorWindow != nullptr) {
        _module->_editorWindow->setSize(newSize->getWidth(), newSize->getHeight());
    }
    return kResultTrue;
}

