#include "Column.h"
#include <algorithm>
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "Composer.h"
#include "GuiUtil.h"
#include "Line.h"
#include "Track.h"

Column::Column(Line* line) : _line(line) {
}

Column::Column(const char* note, unsigned char velocity, unsigned char delay, Line* line) :
    _note(note), _velocity(velocity), _delay(delay),
    _lastNote(note), _lastVelocity(velocity), _lastDelay(delay),
    _line(line) {
}

void Column::render() {
    ImGui::PushID(this);

    ImGui::SetNextItemWidth(widthWithPadding(3));
    if (ImGui::InputText("##note", &_note, ImGuiInputTextFlags_AutoSelectAll)) {
        std::transform(_note.begin(), _note.end(), _note.begin(),
                       [](auto c) { return static_cast<char>(std::toupper(c)); });
    }
    bool activep = ImGui::IsItemActive();
    if (_noteEditing && !activep) {
        if (_note != _lastNote) {
            _line->_track->_composer->_commandManager.executeCommand(
                new EditProperty<Column, std::string>(this, &Column::_note, &Column::_lastNote,
                                                      _note, _lastNote));
        }
    }
    _noteEditing = activep;

    ImGui::SetNextItemWidth(widthWithPadding(2));
    ImGui::SameLine();
    if (ImGui::InputScalar("##velocity", ImGuiDataType_U8, &_velocity, nullptr, nullptr, "%02X")) {
        if (_velocity > 0x7f) {
            _velocity = 0x7f;
        }
    }
    activep = ImGui::IsItemActive();
    if (_velocityEditing && !activep) {
        if (_velocity != _lastVelocity) {
            _line->_track->_composer->_commandManager.executeCommand(
                new EditProperty<Column, unsigned char>(this, &Column::_velocity, &Column::_lastVelocity,
                                                        _velocity, _lastVelocity));
        }
    }
    _velocityEditing = activep;

    ImGui::SetNextItemWidth(widthWithPadding(2));
    ImGui::SameLine();
    ImGui::InputScalar("##delay", ImGuiDataType_U8, &_delay, nullptr, nullptr, "%02X");
    activep = ImGui::IsItemActive();
    if (_delayEditing && !activep) {
        if (_delay != _lastDelay) {
            _line->_track->_composer->_commandManager.executeCommand(
                new EditProperty<Column, unsigned char>(this, &Column::_delay, &Column::_lastDelay, _delay, _lastDelay));
        }
    }
    _delayEditing = activep;

    ImGui::PopID();
}
