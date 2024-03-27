#include "ModuleConnect.h"
#include "../App.h"
#include "../Composer.h"

command::ModuleConnect::ModuleConnect(Module* from, int fromIndex, Module* to, int toIndex, bool post) :
    _fromNekoRef(from->getNekoId()), _fromIndex(fromIndex),
    _toNekoRef(to->getNekoId()), _toIndex(toIndex),
    _post(post)
{
}

void command::ModuleConnect::execute(Composer* composer)
{
    Module* from = Neko::findByNekoId<Module>(_fromNekoRef);
    Module* to = Neko::findByNekoId<Module>(_toNekoRef);
    if (!from || !to) return;

    {
        std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
        to->connect(from, _fromIndex, _toIndex, _post);
        composer->computeProcessOrder();
    }
}

void command::ModuleConnect::undo(Composer* composer)
{
    Module* from = Neko::findByNekoId<Module>(_fromNekoRef);
    Module* to = Neko::findByNekoId<Module>(_toNekoRef);
    if (!from || !to) return;

    {
        std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
        to->disconnect(from, _fromIndex, _toIndex);
        composer->computeProcessOrder();
    }
}
