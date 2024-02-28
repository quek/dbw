#include "CutTracks.h"
#include "../App.h"
#include "../Composer.h"
#include "../Track.h"

command::CutTracks::CutTracks(std::vector<Track*>& tracks) : DeleteTracks(tracks) {
}

void command::CutTracks::execute(Composer* composer) {
    DeleteTracks::execute(composer);
    ImGui::SetClipboardText(_jsonTracks.dump(2).c_str());
}

void command::CutTracks::undo(Composer* composer) {
    DeleteTracks::undo(composer);
}
