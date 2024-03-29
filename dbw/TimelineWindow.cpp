#include "TimelineWindow.h"
#include <mutex>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "App.h"
#include "AudioClip.h"
#include "AutomationClip.h"
#include "AutomationTarget.h"
#include "Clip.h"
#include "Composer.h"
#include "Grid.h"
#include "GuiUtil.h"
#include "Lane.h"
#include "NoteClip.h"
#include "TrackHeaderView.h"
#include "command/AddClips.h"
#include "command/AddTrack.h"
#include "command/DeleteClips.h"
#include "command/DuplicateClips.h"

constexpr float TIMELINE_START_OFFSET = 10.0f;
constexpr float TIMELINE_WIDTH = 20.0f;

TimelineWindow::TimelineWindow(Composer* composer) :
    TimelineCanvasMixin(composer),
    _trackWidthManager(composer->_masterTrack.get()),
    _trackHeaderView(composer, _trackWidthManager)
{
    _show = true;
    _zoomX = 1.0f;
    _zoomY = 40.0f;
    _grid = gGrids[0].get();
}

void TimelineWindow::handleMove(double oldTime, double newTime, Lane* oldLane, Lane* newLane)
{
    double timeDelta = newTime - oldTime;

    auto oldIndex = std::distance(_allLanes.begin(), std::find(_allLanes.begin(), _allLanes.end(), oldLane));
    auto newIndex = std::distance(_allLanes.begin(), std::find(_allLanes.begin(), _allLanes.end(), newLane));
    auto indexDelta = newIndex - oldIndex;

    for (auto clip : _state._draggingThings)
    {
        clip->_time += timeDelta;

        if (indexDelta == 0)
        {
            continue;
        }
        Lane* from = _clipLaneMap[clip];
        auto fromIndex = std::distance(_allLanes.begin(), std::find(_allLanes.begin(), _allLanes.end(), from));
        auto toIndex = fromIndex + indexDelta;
        if (toIndex < 0 || toIndex >= static_cast<int>(_allLanes.size()))
        {
            continue;
        }
        Lane* to = _allLanes[toIndex];
        auto it = std::ranges::find_if(from->_clips, [clip](const auto& x) { return x.get() == clip; });
        to->_clips.push_back(std::move(*it));
        from->_clips.erase(it);
    }
}

void TimelineWindow::handleMouse(const ImVec2& clipRectMin, const ImVec2& clipRectMax)
{
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mousePos = io.MousePos;
    if (!Bounds(clipRectMin, clipRectMax).contains(mousePos))
    {
        return;
    }
    const ImGuiPayload* payload = ImGui::GetDragDropPayload();
    if (payload && payload->IsDataType(DDP_SEQUENCE_MATRIX_CLIPS))
    {
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            Clip* clip = ((Clip*)payload->Data)->clone();
            float time = timeFromMousePos(0.0f, false);
            Lane* lane = laneFromPos(mousePos);
            clip->_time = time;
            lane->_clips.emplace_back(clip);
        }
    } else if (payload && payload->IsDataType(DDP_EXTERNAL_FILES))
    {
        ImVec2 pos = mousePos - ImGui::GetWindowPos() - ImVec2(10, 10);
        ImGui::SetCursorPos(pos);
        ImGui::InvisibleButton(DDP_EXTERNAL_FILES, ImVec2(50, 50));
        if (ImGui::BeginDragDropTarget())
        {
            if (ImGui::AcceptDragDropPayload(DDP_EXTERNAL_FILES))  // or: const ImGuiPayload* payload = ... if you sent a payload in the block above
            {
                auto& files = _composer->app()->getDropFiles();
                auto& file = files[0];

            float time = timeFromMousePos(0.0f, false);
            Lane* lane = laneFromPos(mousePos);
            AudioClip* clip = new AudioClip(time, file, _composer->_bpm);
            lane->_clips.emplace_back(clip);
            }

            ImGui::EndDragDropTarget();
        }
    }

    TimelineCanvasMixin::handleMouse(clipRectMin, clipRectMax);
}

std::pair<std::set<Clip*>, Command*> TimelineWindow::copyThings(std::set<Clip*> srcs, bool redoable)
{
    std::set<Clip*> clips;
    std::set<std::pair<Lane*, Clip*>> targets;
    for (auto& it : srcs)
    {
        Clip* clip = it->clone();
        clips.insert(clip);
        targets.insert(std::pair(_clipLaneMap[it], clip));
    }
    return { clips,new command::AddClips(targets, redoable) };
}

Command* TimelineWindow::deleteThings(std::set<Clip*>& clips, bool undoable)
{
    std::set<std::pair<Lane*, Clip*>> targets;
    for (auto& it : clips)
    {
        targets.insert(std::pair(_clipLaneMap[it], it));
    }
    return new command::DeleteClips(targets, undoable);
}

Command* TimelineWindow::duplicateThings(std::set<Clip*>& clips, bool undoable)
{
    std::set<std::pair<Lane*, Clip*>> targets;
    for (auto& it : clips)
    {
        targets.insert(std::pair(_clipLaneMap[it], it));
    }
    return new command::DuplicateClips(targets, undoable);
}

void TimelineWindow::handleDoubleClick(Clip* clip)
{
    clip->edit(_composer, _clipLaneMap[clip]);
}

Clip* TimelineWindow::handleDoubleClick(double time, Lane* lane)
{
    Clip* clip;
    if (lane->_automationTarget)
    {
        clip = new AutomationClip(time);
    } else
    {
        clip = new NoteClip(time);
    }
    std::set<std::pair<Lane*, Clip*>> clips;
    clips.insert(std::pair(lane, clip));
    _composer->commandExecute(new command::AddClips(clips, true));
    return clip;
}

void TimelineWindow::prepareAllThings()
{
    _allThings.clear();
    _allLanes.clear();
    _clipLaneMap.clear();
    prepareAllThings(_composer->_masterTrack.get());
}

void TimelineWindow::prepareAllThings(Track* track)
{
    for (auto& lane : track->_lanes)
    {
        _allLanes.push_back(lane.get());
        for (auto& clip : lane->_clips)
        {
            _allThings.push_back(clip.get());
            _clipLaneMap[clip.get()] = lane.get();
        }
    }
    for (const auto& x : track->getTracks())
    {
        prepareAllThings(x.get());
    }
}

void TimelineWindow::renderThing(Clip* clip, const ImVec2& pos1, const ImVec2& pos2)
{
    TimelineCanvasMixin::renderThing(clip, pos1, pos2);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddText(pos1, IM_COL32_WHITE, clip->name().c_str());
}

float TimelineWindow::offsetTop() const
{
    return _headerHeight;
}

float TimelineWindow::offsetLeft() const
{
    return TIMELINE_WIDTH;
}

float TimelineWindow::offsetStart() const
{
    //return TIMELINE_START_OFFSET;
    return 0.0f;
}

float TimelineWindow::xFromThing(Clip* clip)
{
    return laneToScreenX(_clipLaneMap[clip]);
}

float TimelineWindow::laneToScreenX(Lane* lane)
{
    float x = 0.0f;
    for (auto it : _allLanes)
    {
        if (it == lane)
        {
            break;
        }
        if (it == _allLanes.back())
        {
            break;
        }
        x += _trackWidthManager.getLaneWidth(it);
    }
    return x * _zoomX + offsetLeft() - ImGui::GetScrollX() + ImGui::GetWindowPos().x;
}

void TimelineWindow::handleShortcut()
{
    if (defineShortcut(ImGuiMod_Ctrl | ImGuiKey_T))
    {
        _composer->commandExecute(new command::AddTrack());
    }

    TimelineCanvasMixin::handleShortcut();
}

void TimelineWindow::renderPlayhead()
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();
    float scrollY = ImGui::GetScrollY();
    float y = (_composer->_playTime * _zoomY) + offsetTop() + offsetStart();
    if (_composer->_playing && _composer->_isScrollFollowPlayhead && !_composer->_scrollLock)
    {
        if (y < ImGui::GetWindowHeight() / 2.0f)
        {
            ImGui::SetScrollY(0);
        } else
        {
            ImGui::SetScrollY(y - ImGui::GetWindowHeight() / 2.0f);
        }
        y = std::min(y, ImGui::GetWindowHeight() / 2.0f);
    } else
    {
        y -= scrollY;
    }
    ImVec2 pos1 = ImVec2(0.0f, y) + windowPos;
    ImVec2 pos2 = ImVec2(ImGui::GetWindowWidth(), y) + windowPos;
    drawList->AddLine(pos1, pos2, PLAY_CURSOR_COLOR);
}

void TimelineWindow::renderHeader()
{
    _headerHeight = _trackHeaderView.render(offsetLeft(), _zoomX);
}

std::string TimelineWindow::windowName()
{
    return "Timeline";
    //return _composer->_project->_name.string() + "##Timeline";
}

std::string TimelineWindow::canvasName()
{
    return "##Timeline Canvas";
}

float TimelineWindow::getLaneWidth(Clip* clip)
{
    Lane* lane = _clipLaneMap[clip];
    return _trackWidthManager.getLaneWidth(lane);
}

Lane* TimelineWindow::laneFromPos(ImVec2& pos)
{
    ImVec2 windowPos = ImGui::GetWindowPos();
    float scrollX = ImGui::GetScrollX();
    float x = (pos.x - windowPos.x + scrollX - TIMELINE_WIDTH) / _zoomX;
    for (auto& lane : _composer->_masterTrack->_lanes)
    {
        float laneWidth = _trackWidthManager.getLaneWidth(lane.get());
        if (laneWidth > x)
        {
            return lane.get();
        }
        x -= laneWidth;
    }
    for (auto& track : _composer->_masterTrack->getTracks())
    {
        for (auto& lane : track->_lanes)
        {
            float laneWidth = _trackWidthManager.getLaneWidth(lane.get());
            if (laneWidth > x)
            {
                return lane.get();
            }
            x -= laneWidth;
        }
    }
    return nullptr;
}
