#pragma once
class Composer;

class SceneMatrix {
public:
    SceneMatrix(Composer* composer);
    Composer* composer() { return _composer; }
    void render();
private:
    Composer* _composer;
};
