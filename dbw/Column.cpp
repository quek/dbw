#include "Column.h"
#include <algorithm>
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "Composer.h"
#include "GuiUtil.h"
#include "Line.h"
#include "Track.h"

Column::Column(Line* line) : _line(line)
{
}

Column::Column(const char* note, unsigned char velocity, unsigned char delay, Line* line) :
    _note(note), _velocity(velocity), _delay(delay),
    _lastNote(note), _lastVelocity(velocity), _lastDelay(delay),
    _line(line)
{
}

class EditVelocity : public Command {
public:
    EditVelocity(Column* column, unsigned char velocity, unsigned char lastVelocity) : _column(column), _velocity(velocity), _lastVelocity(lastVelocity) {}

    void execute(Composer* /*composer*/) override {
        _column->_velocity = _velocity;
        _column->_lastVelocity = _velocity;
    }

    void undo(Composer* /*composer*/) override {
        _column->_velocity = _lastVelocity;
        _column->_lastVelocity = _lastVelocity;
    }

    Column* _column;
    unsigned char _velocity;
    unsigned char _lastVelocity;
};

void Column::render()
{
    ImGui::PushID(this);
    ImGui::SetNextItemWidth(widthWithPadding(3));
    if (ImGui::InputText("##note", &_note)) {
        std::transform(_note.begin(), _note.end(), _note.begin(),
            [](auto c) { return static_cast<char>(std::toupper(c)); });
    }
    ImGui::SetNextItemWidth(widthWithPadding(2));
    ImGui::SameLine();
    if (ImGui::InputScalar("##velocity", ImGuiDataType_U8, &_velocity, nullptr, nullptr, "%02X")) {
        if (_velocity > 0x7f) {
            _velocity = 0x7f;
        }
    }
    bool activep = ImGui::IsItemActive();
    if (_velocityEditing && !activep) {
        if (_velocity != _lastVelocity) {
            _line->_track->_composer->_commandManager.executeCommand(new EditVelocity(this, _velocity, _lastVelocity));
        }
    }
    _velocityEditing = activep;

    ImGui::SetNextItemWidth(widthWithPadding(2));
    ImGui::SameLine();
    ImGui::InputScalar("##delay", ImGuiDataType_U8, &_delay, nullptr, nullptr, "%02X");
    ImGui::PopID();
}
