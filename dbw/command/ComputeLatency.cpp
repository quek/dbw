#include "ComputeLatency.h"
#include "../Composer.h"

command::ComputeLatency::ComputeLatency() : Command(false) {
}

void command::ComputeLatency::execute(Composer* composer) {
    composer->computeLatency();
}

void command::ComputeLatency::undo(Composer* composer) {
}
