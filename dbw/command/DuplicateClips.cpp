#include "DuplicateClips.h"

command::DuplicateClips::DuplicateClips(std::set<std::pair<Lane*, Clip*>>& targets, bool undoable) :
    Command(undoable){
}

void command::DuplicateClips::execute(Composer* composer) {
}

void command::DuplicateClips::undo(Composer* composer) {
}
