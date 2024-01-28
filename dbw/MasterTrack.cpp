#include "MasterTrack.h"
#include "imgui.h"
#include "Column.h"
#include "Composer.h"
#include "Line.h"
#include "Module.h"
#include "PlayPosition.h"

MasterTrack::MasterTrack(Composer* composer) : Track("MASTER", composer) {
}

void MasterTrack::process(const ProcessBuffer* in, unsigned long framesPerBuffer, int64_t steadyTime) {
    PlayPosition* from = &_composer->_playPosition;
    PlayPosition* to = &_composer->_nextPlayPosition;
    int toLine = to->_delay == 0 ? to->_line : to->_line + 1;
    for (int lineIndex = from->_line; lineIndex <= toLine && lineIndex < _lines.size(); ++lineIndex) {
        auto& line = _lines[lineIndex];
        for (int columnIndex = 0; columnIndex < _ncolumns; ++columnIndex) {
            auto& column = line->_columns[columnIndex];
            auto lastKey = _lastKeys[columnIndex];
            PlayPosition linePosition{ ._line = lineIndex, ._delay = static_cast<unsigned char>(column->_delay) };
            if (linePosition < *from || *to <= linePosition) {
                continue;
            }
            auto delay = linePosition.diffInDelay(*from);
            uint32_t sampleOffset = delay * _composer->_samplePerDelay;

            if (column->_note.empty()) {
                continue;
            }
            int16_t key = noteToNumber(column->_note);
            if (key == NOTE_NONE) {
                continue;
            }
            if (key == NOTE_OFF) {
                _processBuffer._eventIn.noteOff(lastKey, 0, 0x7f, sampleOffset);
                continue;
            }
            if (lastKey != NOTE_NONE) {
                _processBuffer._eventIn.noteOff(lastKey, 0, 0x7f, sampleOffset);
            }
            _processBuffer._eventIn.noteOn(key, 0, column->_velocity, sampleOffset);
            _lastKeys[columnIndex] = key;
        }
    }

    ProcessBuffer* buffer = &_processBuffer;
    for (auto module = _modules.begin(); module != _modules.end(); ++module) {
        (*module)->process(buffer, framesPerBuffer, steadyTime);
        buffer = &(*module)->_processBuffer;
    }
    _processBuffer.copyOutToOutFrom(buffer);

}

void MasterTrack::renderLine(int line) {
    ImGui::Text("--");
}

