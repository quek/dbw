#include "SceneMatrix.h"
#include <mutex>
#include <ranges>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "App.h"
#include "Clip.h"
#include "ClipSlot.h"
#include "Composer.h"
#include "Config.h"
#include "Lane.h"
#include "NoteClip.h"

constexpr float PLAY_STOP_BUTTON_WIDTH = 20.0f;

SceneMatrix::SceneMatrix(const nlohmann::json& json, SerializeContext& context) :
    Nameable(json, context)
{
    for (const auto& x : json["_scenes"])
    {
        Scene* scene = new Scene(x, context);
        scene->_sceneMatrix = this;
        _scenes.emplace_back(scene);
    }
}

SceneMatrix::SceneMatrix(Composer* composer) :
    _composer(composer)
{
    _trackWidthManager.reset(new TrackWidthManager(composer->_masterTrack.get()));
    _trackHeaderView.reset(new TrackHeaderView(composer, *_trackWidthManager));
    addScene(false);
}

void SceneMatrix::render()
{
    if (ImGui::Begin("Scene Matrix"))
    {
        _headerHeight = _trackHeaderView->render(offsetX(), _zoomX);
        ImGui::SetCursorPos(ImVec2(ImGui::GetStyle().WindowPadding.x, _headerHeight));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
        for (auto& scene : _scenes)
        {
            renderScene(scene.get());
        }
        renderSceneAdd();
        ImGui::PopStyleVar();

        if (false)
        {
            int ncolumns = 1;
            for (const auto& track : _composer->allTracks())
            {
                ncolumns += static_cast<int>(track->_lanes.size());
            }
            if (ImGui::BeginTable("Scene Matrix Table", ncolumns,
                                  ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY,
                                  ImVec2(-1.0f, -20.0f)))
            {
                ImGui::TableSetupScrollFreeze(1, 1);
                ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_NoHide);
                ImGui::TableSetupColumn(_composer->_masterTrack->_name.c_str());
                for (auto& track : _composer->_masterTrack->getTracks())
                {
                    ImGui::TableSetupColumn(track->_name.c_str());
                }

                ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                ImGui::TableSetColumnIndex(0);
                ImGui::TableHeader("");

                ImGui::TableSetColumnIndex(1);
                ImGui::PushID(_composer->_masterTrack.get());
                ImGui::TableHeader(_composer->_masterTrack->_name.c_str());
                ImGui::PopID();

                int columnIndex = 1;
                for (auto& track : _composer->_masterTrack->getTracks())
                {
                    ImGui::TableSetColumnIndex(++columnIndex);
                    ImGui::PushID(track.get());
                    auto name = track->_name.c_str();
                    ImGui::TableHeader(name);
                    ImGui::PopID();
                }

                for (const auto& scene : _scenes)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                    ImGui::BeginDisabled(scene->isAllLanePlaying());
                    if (ImGui::Button("▶"))
                    {
                        scene->play();
                    }
                    ImGui::EndDisabled();
                    ImGui::SameLine();
                    ImGui::BeginDisabled(scene->isAllLaneStoped());
                    if (ImGui::Button("■"))
                    {
                        scene->stop();
                    }
                    ImGui::EndDisabled();
                    ImGui::PopStyleVar();
                    ImGui::SameLine();
                    ImGui::Text(scene->_name.c_str());

                    columnIndex = 0;
                    ImGui::TableSetColumnIndex(++columnIndex);
                    // TODO group tracks
                    for (const auto& lane : _composer->_masterTrack->_lanes)
                    {
                        auto& clipSlot = lane->getClipSlot(scene.get());
                        clipSlot->render(_composer);
                        if (ImGui::BeginDragDropTarget())
                        {
                            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(DDP_SEQUENCE_MATRIX_CLIPS))
                            {
                                Clip* clip = (Clip*)payload->Data;
                                clipSlot->_clip.reset(clip->clone());
                            }
                            ImGui::EndDragDropTarget();
                        }
                    }
                    for (const auto& track : _composer->_masterTrack->getTracks())
                    {
                        for (const auto& lane : track->_lanes)
                        {
                            ImGui::TableSetColumnIndex(++columnIndex);
                            auto& clipSlot = lane->getClipSlot(scene.get());
                            clipSlot->render(_composer);
                            if (ImGui::BeginDragDropTarget())
                            {
                                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(DDP_SEQUENCE_MATRIX_CLIPS))
                                {
                                    Clip* clip = (Clip*)payload->Data;
                                    clipSlot->_clip.reset(clip->clone());
                                }
                                ImGui::EndDragDropTarget();
                            }
                        }
                    }
                }
                ImGui::EndTable();
            }

            if (ImGui::Button("Add Scene##addScene"))
            {
                addScene();
            }
        }
    }
    ImGui::End();
}

void SceneMatrix::process(Track* track)
{
    double oneBeatSec = 60.0 / track->getComposer()->_bpm;
    for (auto& scene : _scenes)
    {
        for (auto& lane : track->_lanes)
        {
            auto& clipSlot = lane->getClipSlot(scene.get());
            if (!clipSlot->_playing)
            {
                continue;
            }
            if (!clipSlot->_clip)
            {
                continue;
            }
            double sequenceDuration = clipSlot->_clip->_sequence->durationGet();
            double begin = fmod(_composer->_playTime, sequenceDuration);
            double end = fmod(_composer->_nextPlayTime, sequenceDuration);
            for (auto& item : clipSlot->_clip->_sequence->getItems())
            {
                item->prepareProcessBuffer(lane.get(), begin, end, 0.0, sequenceDuration, 0.0, sequenceDuration, 0.0, sequenceDuration, oneBeatSec);
            }
        }
    }
}

void SceneMatrix::stop()
{
    for (auto& scene : _scenes)
    {
        scene->stop();
    }
}

void SceneMatrix::addScene(bool undoable)
{
    _composer->commandExecute(new AddSceneCommand(this, undoable));
}

Composer* SceneMatrix::composerGet()
{
    return _composer;
}

void SceneMatrix::composerSet(Composer* composer)
{
    _composer = composer;
    _trackWidthManager.reset(new TrackWidthManager(composer->_masterTrack.get()));
    _trackHeaderView.reset(new TrackHeaderView(composer, *_trackWidthManager));
}

nlohmann::json SceneMatrix::toJson(SerializeContext& context)
{
    nlohmann::json json = Nameable::toJson(context);

    nlohmann::json scenes = nlohmann::json::array();
    for (const auto& scene : _scenes)
    {
        scenes.emplace_back(scene->toJson(context));
    }
    json["_scenes"] = scenes;

    return json;
}

void SceneMatrix::dragDropTarget(std::unique_ptr<Clip>& clip)
{
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(DDP_SEQUENCE_MATRIX_CLIPS))
        {
            Clip* dragClip = (Clip*)payload->Data;
            clip.reset(dragClip->clone());
        }
        ImGui::EndDragDropTarget();
    }
}

float SceneMatrix::offsetX()
{
    return 100.0f;
}

void SceneMatrix::renderScene(Scene* scene)
{
    ImGui::PushID(scene);
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos1 = windowToScreen(ImGui::GetCursorPos());
    ImVec2 pos2 = pos1 + ImVec2(ImGui::GetWindowWidth(), 0.0f);
    drawList->AddLine(pos1, pos2, gTheme.rackBorder);

    auto& style = ImGui::GetStyle();
    float width = offsetX() - style.WindowPadding.x - PLAY_STOP_BUTTON_WIDTH * 2;
    ImGui::Button(scene->_name.c_str(), ImVec2(width, 0.0f));

    ImGui::SameLine();
    ImGui::BeginDisabled(scene->isAllLanePlaying());
    if (ImGui::Button("▶", ImVec2(PLAY_STOP_BUTTON_WIDTH, 0.0f)))
    {
        scene->play();
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::BeginDisabled(scene->isAllLaneStoped());
    if (ImGui::Button("■", ImVec2(PLAY_STOP_BUTTON_WIDTH, 0.0f)))
    {
        scene->stop();
    }
    ImGui::EndDisabled();

    renderSceneTrack(scene, _composer->_masterTrack.get());
    ImGui::PopID();
}

void SceneMatrix::renderSceneAdd()
{
    ImGui::PushID("renderSceneAdd");
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos1 = windowToScreen(ImGui::GetCursorPos());
    ImVec2 pos2 = pos1 + ImVec2(ImGui::GetWindowWidth(), 0.0f);
    drawList->AddLine(pos1, pos2, gTheme.rackBorder);

    auto& style = ImGui::GetStyle();
    float width = offsetX() - style.WindowPadding.x;
    if (ImGui::Button("+", ImVec2(width, 0.0f)))
    {
        addScene();
    }
    ImGui::PopID();
}

void SceneMatrix::renderSceneTrack(Scene* scene, Track* track)
{
    for (auto& lane : track->_lanes)
    {
        renderSceneTrackLane(scene, track, lane.get());
    }

    if (track->_showTracks)
    {
        for (auto& x : track->getTracks())
        {
            renderSceneTrack(scene, x.get());
        }
    }
}

void SceneMatrix::renderSceneTrackLane(Scene* scene, Track*, Lane* lane)
{
    ImGui::PushID(lane);
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32_BLACK_TRANS);
    ImGui::SameLine();
    auto& clip = lane->getClipSlot(scene)->_clip;
    float width = _trackHeaderView->getLaneWidth(lane);
    if (clip)
    {
        width -= PLAY_STOP_BUTTON_WIDTH;
        ImGui::Button(clip->name().c_str(), ImVec2(width, 0.0f));
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            clip->edit(_composer, lane);
        }
        dragDropTarget(clip);
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            ImGui::SetDragDropPayload(DDP_SEQUENCE_MATRIX_CLIPS, clip.get(), sizeof(*(clip.get())));
            ImGui::Text(clip->name().c_str());
            ImGui::EndDragDropSource();
        }

        ImGui::SameLine();
        if (!lane->getClipSlot(scene)->_playing)
        {
            if (ImGui::Button("▶", ImVec2(PLAY_STOP_BUTTON_WIDTH, 0.0f)))
            {
                lane->getClipSlot(scene)->play();
            }
        }
        else
        {
            if (ImGui::Button("■", ImVec2(PLAY_STOP_BUTTON_WIDTH, 0.0f)))
            {
                lane->getClipSlot(scene)->stop();
            }
        }
    }
    else
    {
        width -= PLAY_STOP_BUTTON_WIDTH;
        if (ImGui::Button("+", ImVec2(width, 0.0f)))
        {
            // TODO undo
            clip.reset(new NoteClip());
        }
        dragDropTarget(clip);
        ImGui::SameLine();
        if (ImGui::Button("●", ImVec2(PLAY_STOP_BUTTON_WIDTH, 0.0f)))
        {
            // TODO rec
        }
    }
    ImGui::PopStyleColor();
    ImGui::PopID();
}

AddSceneCommand::AddSceneCommand(SceneMatrix* sceneMatrix, bool undoable) : _sceneMatrix(sceneMatrix), Command(undoable)
{
}

void AddSceneCommand::execute(Composer* composer)
{
    if (!_scene)
    {
        _scene = std::make_unique<Scene>(_sceneMatrix);
    }
    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
    _sceneMatrix->_scenes.push_back(std::move(_scene));
}

void AddSceneCommand::undo(Composer* composer)
{
    std::lock_guard<std::recursive_mutex> lock(composer->app()->_mtx);
    _scene = std::move(_sceneMatrix->_scenes.back());
    _sceneMatrix->_scenes.pop_back();
}
