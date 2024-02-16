#pragma once

class Composer;
class Module;

class SidechainInputSelector {
public:
    SidechainInputSelector(Composer* composer);
    static constexpr const char* NAME = "Sidechain Input##Sidechain Input";
    void render();
    void open(Module* module);
private:
    Composer* _composer;
    Module* _module = nullptr;
    bool _show = false;
};

