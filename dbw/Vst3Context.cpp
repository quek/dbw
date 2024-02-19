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
    // TODO
    Error("beginEdit {}", id);
    return kResultOk;
}

tresult PLUGIN_API Vst3Context::performEdit(Vst::ParamID id, Vst::ParamValue valueNormalized) {
    // TODO
    Error("performEdit {} {}", id, valueNormalized);
    return kResultOk;
}

tresult PLUGIN_API Vst3Context::endEdit(Vst::ParamID id) {
    // TODO
    Error("endEdit {}", id);
    return kResultOk;
}

tresult PLUGIN_API Vst3Context::restartComponent(int32 flags) {
    // TODO
    Error("restartComponent {}", flags);
    _module->stop();
    _module->start();
    return kResultOk;
}

tresult PLUGIN_API Vst3Context::resizeView(IPlugView* /*view*/, ViewRect* newSize) {
    if (_module->_editorWindow != nullptr) {
        _module->_editorWindow->setSize(newSize->getWidth(), newSize->getHeight());
    }
    return kResultTrue;
}

