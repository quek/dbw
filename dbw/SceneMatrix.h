#pragma once
#include <memory>
#include <vector>
#include "Command.h"
#include "Nameable.h"
#include "Scene.h"

class Composer;
class Track;

class SceneMatrix : public Nameable {
public:
    SceneMatrix(const nlohmann::json& json);
    SceneMatrix(Composer* composer);
    Composer* composer() const { return _composer; }
    void render();
    void process(Track* track);
    void stop();
    void addScene(bool undoable = true);
    virtual nlohmann::json toJson() override;

    std::vector<std::unique_ptr<Scene>> _scenes;
    Composer* _composer = nullptr;
};

class AddSceneCommand : public Command {
public:
    AddSceneCommand(SceneMatrix* sceneMatrix, bool undable = true);
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;
    SceneMatrix* _sceneMatrix;
    std::unique_ptr<Scene> _scene;
};
