#include "Line.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <algorithm>
#include "imgui.h"
#include "Composer.h"
#include "GuiUtil.h"

Line::Line(Track* track) : _track(track)
{
}

Line::Line(const char* note, unsigned char velocity, unsigned char delay, Track* track) :
    _note(note), _velocity(velocity), _delay(delay),
    _lastNote(note), _lastVelocity(velocity), _lastDelay(delay),
    _track(track)
{
}

void Line::render()
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
        unsigned char value = _velocity;
        unsigned char lastValue = _lastVelocity;
        if (_velocity != _lastVelocity) {
            auto execute = [this, value](Composer* composer) {
                _velocity = value;
                _lastVelocity = value;
                };
            auto undo = [this, lastValue](Composer* composer) {
                _velocity = lastValue;
                _lastVelocity = lastValue;
                };
            _track->_composer->_commandManager.executeCommand(std::make_shared<Command>(execute, undo));
        }
    }
    _velocityEditing = activep;

    ImGui::SetNextItemWidth(widthWithPadding(2));
    ImGui::SameLine();
    ImGui::InputScalar("##delay", ImGuiDataType_U8, &_delay, nullptr, nullptr, "%02X");
    ImGui::PopID();
}
