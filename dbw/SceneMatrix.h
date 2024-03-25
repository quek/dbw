#pragma once
#include <memory>
#include <vector>
#include "BaseWindow.h"
#include "Clip.h"
#include "Command.h"
#include "Nameable.h"
#include "Scene.h"
#include "TrackHeaderView.h"
#include "TrackWidthManager.h"
#include "ZoomMixin.h"

class Composer;
class Track;

class SceneMatrix : public Nameable, public BaseWindow, public ZoomMixin
{
public:
    SceneMatrix(const nlohmann::json& json, SerializeContext& context);
    SceneMatrix(Composer* composer);
    Composer* composer() const { return _composer; }
    void render();
    void process(Track* track);
    void stop();
    void addScene(bool undoable = true);
    Composer* composerGet();
    void composerSet(Composer* composer);
    virtual nlohmann::json toJson(SerializeContext& context) override;

    std::vector<std::unique_ptr<Scene>> _scenes;

private:
    void dragDropTarget(std::unique_ptr<Clip>& clip);
    float offsetX();
    void renderScene(Scene* scene);
    void renderSceneAdd();
    void renderSceneTrack(Scene* scene, Track* track);
    void renderSceneTrackLane(Scene* scene, Track* track, Lane* lane);

    Composer* _composer = nullptr;
    float _headerHeight = 0.0f;
    std::unique_ptr<TrackWidthManager> _trackWidthManager = nullptr;
    std::unique_ptr<TrackHeaderView> _trackHeaderView = nullptr;
};

class AddSceneCommand : public Command
{
public:
    AddSceneCommand(SceneMatrix* sceneMatrix, bool undable = true);
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;
    SceneMatrix* _sceneMatrix;
    std::unique_ptr<Scene> _scene;
};
