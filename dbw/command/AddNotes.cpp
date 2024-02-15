#include "AddNotes.h"

command::AddNotes::AddNotes(std::vector<std::pair<TrackLane*, Clip*>> notes, bool undoable) : Command(undoable) {
}

void command::AddNotes::execute(Composer* composer) {
}

void command::AddNotes::undo(Composer*) {
}
