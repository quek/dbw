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

template <typename T>
class EditColumn : public Command {
public:
    EditColumn(Column* column, T Column::* x, T Column::* lastX, T value, T lastValue)
        : _column(column), _x(x), _lastX(lastX), _value(value), _lastValue(lastValue) {
    }

    void execute(Composer* /*composer*/) override {
        _column->*_x = _value;
        _column->*_lastX = _value;
    }

    void undo(Composer* /*composer*/) override {
        _column->*_x = _lastValue;
        _column->*_lastX = _lastValue;
    }

private:
    Column* _column;
    T Column::* _x;
    T Column::* _lastX;
    T _value;
    T _lastValue;
};

void Column::render() {
    ImGui::PushID(this);

    ImGui::SetNextItemWidth(widthWithPadding(3));
    if (ImGui::InputText("##note", &_note)) {
        std::transform(_note.begin(), _note.end(), _note.begin(),
                       [](auto c) { return static_cast<char>(std::toupper(c)); });
    }
    bool activep = ImGui::IsItemActive();
    if (_noteEditing && !activep) {
        if (_note != _lastNote) {
            _line->_track->_composer->_commandManager.executeCommand(
                new EditColumn<std::string>(this, &Column::_note, &Column::_lastNote,
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
                new EditColumn<unsigned char>(this, &Column::_velocity, &Column::_lastVelocity,
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
                new EditColumn<unsigned char>(this, &Column::_delay, &Column::_lastDelay, _delay, _lastDelay));
        }
    }
    _delayEditing = activep;

    ImGui::PopID();
}
