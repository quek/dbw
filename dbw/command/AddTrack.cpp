#include "AddTrack.h"
#include "../App.h"
#include "../Composer.h"

void command::AddTrack::execute(Composer* composer) {
    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
    composer->_masterTrack->addTrack();
    composer->computeProcessOrder();
}

void command::AddTrack::undo(Composer* composer) {
    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
    composer->_masterTrack->getTracks().pop_back();
    composer->computeProcessOrder();
}
