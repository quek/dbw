#include "TrackHeaderView.h"
#include "Composer.h"
#include "Config.h"
#include "GuiUtil.h"
#include "Lane.h"

constexpr const float BASE_HEADER_HEIGHT = 22.0f;
constexpr const float HEIGHT_FOR_AUTOMATION = 44.0f;
constexpr const float GROUP_OFFSET_Y = 5.0f;

TrackHeaderView::TrackHeaderView(Composer* composer, TrackWidthManager& trackWidthManager) :
    _composer(composer), _trackWidthManager(trackWidthManager) {
}

float TrackHeaderView::render(float offsetX) {
    _x = offsetX;
    _scrollX = ImGui::GetScrollX();
    _scrollY = ImGui::GetScrollY();
    computeHeaderHeight();
    renderTrack(_composer->_masterTrack.get(), 0);
    return _headerHeight;
}

void TrackHeaderView::computeHeaderHeight() {
    _headerHeight = BASE_HEADER_HEIGHT;
    computeHeaderHeight(_composer->_masterTrack.get(), 0);
}

void TrackHeaderView::computeHeaderHeight(Track* track, int groupLevel) {
    float height = BASE_HEADER_HEIGHT + GROUP_OFFSET_Y * groupLevel;
    if (std::count_if(track->_lanes.begin(), track->_lanes.end(), [](auto& lane) { return lane->_automationTarget != nullptr; })) {
        height += HEIGHT_FOR_AUTOMATION;
    }
    _headerHeight = std::max(_headerHeight, height);
    if (track->_showTracks) {
        for (const auto& x : track->getTracks()) {
            computeHeaderHeight(x.get(), groupLevel + (track->isMasterTrack() ? 0 : 1));
        }
    }
}

ImVec2 TrackHeaderView::posScreenToWindow(const ImVec2& pos) {
    return pos - ImGui::GetWindowPos() + ImVec2(_scrollX, _scrollY);
}

ImVec2 TrackHeaderView::posWindowToScreen(const ImVec2& pos) {
    return pos + ImGui::GetWindowPos() - ImVec2(_scrollX, _scrollY);
}

void TrackHeaderView::renderLane(Lane* lane, int groupLevel) {
    ImGui::PushID(lane);
    ImVec2 pos1(_x, _scrollY + GROUP_OFFSET_Y * groupLevel + BASE_HEADER_HEIGHT);
    ImGui::SetCursorPos(pos1);
    float width = _trackWidthManager.getLaneWidth(lane);

    const ImGuiPayload* payload = ImGui::GetDragDropPayload();
    if (payload && payload->IsDataType(std::format(DDP_AUTOMATION_TARGET, lane->_track->getNekoId()).c_str())) {
        ImGui::InvisibleButton("DragDropTarget", ImVec2(width, ImGui::GetWindowHeight()));
        if (ImGui::BeginDragDropTarget()) {
            if (payload = ImGui::AcceptDragDropPayload(std::format(DDP_AUTOMATION_TARGET, lane->_track->getNekoId()).c_str())) {
                AutomationTarget* automationTarget = (AutomationTarget*)payload->Data;
                lane->_automationTarget.reset(new AutomationTarget(*automationTarget));
                ImGui::EndDragDropTarget();
            }
        }
    }

    auto& automationTarget = lane->_automationTarget;
    if (automationTarget) {
        ImGui::SetCursorPos(pos1);
        ImGui::BeginGroup();
        Module* module = automationTarget->getModule();
        std::string paramName = automationTarget->getParamName();
        std::string label = std::format("{} {}", module->_name, paramName);
        ImGui::Button(label.c_str(), ImVec2(width, 0.0f));
        double defaultValue = automationTarget->getDefaultValue();
        double min = 0.0;
        double max = 1.0;
        std::string format = automationTarget->getParam()->getValueText(defaultValue);
        ImGui::SetNextItemWidth(width);
        if (ImGui::DragScalar("##default value", ImGuiDataType_Double, &defaultValue, 0.01f, &min, &max, format.c_str())) {
            automationTarget->setDefaultValue(defaultValue);
        }
        ImGui::EndGroup();

        pos1 = posWindowToScreen(pos1);
        ImVec2 pos2 = pos1 + ImVec2(0.0f, ImGui::GetWindowHeight());
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddLine(pos1, pos2, gTheme.rackBorder);
    }

    ImGui::PopID();
}

void TrackHeaderView::renderTrack(Track* track, int groupLevel) {
    ImGui::PushID(track);
    ImVec2 pos1(_x, _scrollY + GROUP_OFFSET_Y * groupLevel);
    ImGui::SetCursorPos(pos1);
    float trackWidth = _trackWidthManager.getTrackWidth(track);
    ImGui::Button(track->_name.c_str(), ImVec2(trackWidth, 0.0f));
    if (ImGui::BeginPopupContextItem(track->_name.c_str())) {
        if (ImGui::MenuItem("New Lane", "Ctrl+L")) {
            // TODO command
            track->addLane(new Lane());
        }
        ImGui::EndPopup();
    }
    pos1 = posWindowToScreen(pos1);
    ImVec2 pos2 = pos1 + ImVec2(0.0f, ImGui::GetWindowHeight());
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddLine(pos1, pos2, gTheme.rackBorder);



    for (auto& lane : track->_lanes) {
        renderLane(lane.get(), groupLevel);
        _x += _trackWidthManager.getLaneWidth(lane.get());
    }

    if (track->_showTracks) {
        for (auto& it : track->getTracks()) {
            renderTrack(it.get(), groupLevel + (track->isMasterTrack() ? 0 : 1));
        }
    }

    ImGui::PopID();
}
