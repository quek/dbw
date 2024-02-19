#pragma once
#include <base/source/fobject.h>
#include <pluginterfaces/vst/ivstcomponent.h>
#include <pluginterfaces/vst/ivstaudioprocessor.h>
#include <pluginterfaces/vst/ivstunits.h>
#include <pluginterfaces/vst/ivsteditcontroller.h>
#include <pluginterfaces/vst/ivstpluginterfacesupport.h>
#include <public.sdk/source/vst/hosting/hostclasses.h>
#include <public.sdk/source/vst/hosting/parameterchanges.h>
#include <pluginterfaces/gui/iplugview.h>

class Vst3Module;

using namespace Steinberg;

class Vst3Context :
    public FObject,
    public Vst::IHostApplication,
    public Vst::IComponentHandler,
    //public Vst::IComponentHandler2,
    //public Vst::IUnitHandler,
    //public Vst::IUnitHandler2,
    public IPlugFrame
    //public Vst::IPlugInterfaceSupport
{
public:
    Vst3Context(Vst3Module* module);
    virtual ~Vst3Context();

    //--- IHostApplication ---------------
    tresult PLUGIN_API getName(Vst::String128 name) override;
    tresult PLUGIN_API createInstance(TUID cid, TUID _iid, void** obj) override;

    //! IComponentHandler
    /** To be called before calling a performEdit (e.g. on mouse-click-down event). */
    tresult PLUGIN_API beginEdit(Vst::ParamID id) override;
    /** Called between beginEdit and endEdit to inform the handler that a given parameter has a new value. */
    tresult PLUGIN_API performEdit(Vst::ParamID id, Vst::ParamValue valueNormalized) override;
    /** To be called after calling a performEdit (e.g. on mouse-click-up event). */
    tresult PLUGIN_API endEdit(Vst::ParamID id) override;
    /** Instructs host to restart the component. This should be called in the UI-Thread context!
     \param flags is a combination of RestartFlags */
    tresult PLUGIN_API restartComponent(int32 flags) override;

    // IPlugFrame
    tresult PLUGIN_API resizeView(IPlugView* view, ViewRect* newSize) SMTG_OVERRIDE;


    OBJ_METHODS(Vst3Context, FObject);
    REFCOUNT_METHODS(FObject);

    DEFINE_INTERFACES;
    DEF_INTERFACE(Vst::IHostApplication);
    DEF_INTERFACE(Vst::IComponentHandler);
    //DEF_INTERFACE(Vst::IComponentHandler2);
    //DEF_INTERFACE(Vst::IUnitHandler);
    //DEF_INTERFACE(Vst::IUnitHandler2);
    DEF_INTERFACE(IPlugFrame);
    END_DEFINE_INTERFACES(FObject);

    //PlugInterfaceSupport* getPlugInterfaceSupport () const { return mPlugInterfaceSupport; }

private:
    //IPtr<PlugInterfaceSupport> mPlugInterfaceSupport;
    Vst3Module* _module;
};
