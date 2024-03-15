#include "NoteClip.h"
#include "Composer.h"
#include "PianorollWindow.h"

NoteClip::NoteClip(double time) : Clip(time) {
}

NoteClip::NoteClip(const nlohmann::json& json, SerializeContext& context) : Clip(json, context) {
}

Clip* NoteClip::clone() {
    return new NoteClip(*this);
}

void NoteClip::edit(Composer* composer, Lane*) {
    composer->editNoteClip(this);
}

void NoteClip::renderInScene(PianoRollWindow* pianoRoll) {
    std::string sequenceName = name();

    ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(-4.0f, 1.0f);
    ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeightWithSpacing()) + ImVec2(8.0f, 0.0f);
    ImGui::GetWindowDrawList()->AddRectFilled(pos, pos + size, IM_COL32(120, 120, 120, 255)); // 背景色を描画
    ImGui::PushID(this);
    if (ImGui::Selectable(sequenceName.c_str(), &_selected, ImGuiSelectableFlags_AllowDoubleClick)) {
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            pianoRoll->edit(this);
        }
    }
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
        ImGui::SetDragDropPayload("Sequence Matrix Clip", this, sizeof(*this));
        ImGui::Text(sequenceName.c_str());
        ImGui::EndDragDropSource();
    }
    ImGui::PopID();
}

nlohmann::json NoteClip::toJson(SerializeContext& context) {
    nlohmann::json json = Clip::toJson(context);
    json["type"] = TYPE;
    return json;
}

