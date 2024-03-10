#include "TrackHeaderView.h"
#include "Composer.h"
#include "Lane.h"

constexpr const float BASE_HEADER_HEIGHT = 22.0f;
constexpr const float HEIGHT_FOR_AUTOMATION = 44.0f;
constexpr const float GROUP_OFFSET_Y = 5.0f;

TrackHeaderView::TrackHeaderView(Composer* composer, TrackWidthManager& trackWidthManager) :
    _composer(composer), _trackWidthManager(trackWidthManager) {
}

float TrackHeaderView::render(float offsetX) {
    _x = offsetX;
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

void TrackHeaderView::renderLane(Lane* lane, int groupLevel) {
    auto& automationTarget = lane->_automationTarget;
    if (!automationTarget) {
        return;
    }

    ImGui::PushID(lane);
    ImGui::SetCursorPos(ImVec2(_x, _scrollY + GROUP_OFFSET_Y * groupLevel + 22.0f));
    ImGui::BeginGroup();
    float width = _trackWidthManager.getLaneWidth(lane);
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
    ImGui::PopID();
}

void TrackHeaderView::renderTrack(Track* track, int groupLevel) {
    ImGui::PushID(track);
    ImGuiIO& io = ImGui::GetIO();
    //ImGui::SetCursorPos(ImVec2(_x + 4.0f, scrollY + GROUP_OFFSET_Y * groupLevel));
    //ImGui::Text(track->_name.c_str());
    ImGui::SetCursorPos(ImVec2(_x, _scrollY + GROUP_OFFSET_Y * groupLevel));
    float trackWidth = _trackWidthManager.getTrackWidth(track);
    ImGui::Button(track->_name.c_str(), ImVec2(trackWidth, 0.0f));
    if (ImGui::BeginPopupContextItem(track->_name.c_str())) {
        if (ImGui::MenuItem("New Lane", "Ctrl+L")) {
            // TODO
            track->addLane(new Lane());
        }
        ImGui::EndPopup();
    }

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
