#include "ModuleDisconnect.h"
#include "../App.h"
#include "../Composer.h"

command::ModuleDisconnect::ModuleDisconnect(Connection* connection) :
        _fromNekoRef(connection->_from->getNekoId()), _fromIndex(connection->_fromIndex),
    _toNekoRef(connection->_to->getNekoId()), _toIndex(connection->_toIndex),
    _post(connection->_post)
{
}

void command::ModuleDisconnect::execute(Composer* composer)
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

void command::ModuleDisconnect::undo(Composer* composer)
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
