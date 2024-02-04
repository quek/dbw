#pragma once
#include <memory>
#include <vector>
#include "Command.h"
#include "Scene.h"

class Composer;

class SceneMatrix {
public:
    SceneMatrix(Composer* composer);
    Composer* composer() { return _composer; }
    void render();
    void addScene(bool undoable = true);

    std::vector<std::unique_ptr<Scene>> _scenes;
private:
    Composer* _composer;
};

class AddSceneCommand : public Command {
public:
    AddSceneCommand(SceneMatrix* sceneMatrix, bool undable = true);
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;
    SceneMatrix* _sceneMatrix;
    std::unique_ptr<Scene> _scene;
};
