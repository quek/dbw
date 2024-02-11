#include "MasterTrack.h"
#include "imgui.h"
#include "Composer.h"
#include "Module.h"

MasterTrack::MasterTrack(Composer* composer) : Track("MASTER", composer) {
}

void MasterTrack::process(int64_t steadyTime) {
    for (auto module = _modules.begin(); module != _modules.end(); ++module) {
        (*module)->process(&_processBuffer, steadyTime);
        _processBuffer.swapInOut();
    }
    _processBuffer.swapInOut();
}
