#pragma once
#include <memory>
#include <mutex>
#include <vector>
#include "AudioEngine.h"
#include "AudioEngineWindow.h"
#include "Composer.h"
#include "MidiDevice.h"

class DropManager;

class App
{
public:
    App();
    virtual ~App();
    void render();
    AudioEngine* audioEngine() const { return _audioEngine.get(); }
    std::vector<std::unique_ptr<Composer>>& composers() { return _composers; }
    void addComposer(Composer* composer);
    void deleteComposer(Composer* composer);
    void dragEnter(const std::vector<std::string>& files);
    void drop();
    std::vector<std::string>& getDropFiles() { return _dropFiles; }
    std::vector<std::unique_ptr<MidiDevice>>& getMidiInDevices() { return _midiInDevices; }
    void runCommand();
    void requestAddComposer(Composer* composer);
    void requestDeleteComposer(Composer* composer);
    void showAudioSetupWindow();
    bool isDragging();
    bool isStarted() const { return _isStarted; }
    void start();
    void stop();
    void openMidiDevices();
    void closeMidiDevices();
    void processMidiDevices();

    std::recursive_mutex _mtx;

private:
    bool _isStarted = false;
    std::unique_ptr<AudioEngine> _audioEngine;
    std::unique_ptr<AudioEngineWindow> _audioEngineWindow = nullptr;
    std::vector<std::unique_ptr<Composer>> _composers;

    DropManager* _dropManager = nullptr;
    bool _isDragging = false;
    std::vector<std::string> _dropFiles;

    std::vector<Composer*> _requestAddComposers;
    std::vector<Composer*> _requestDeleteComposers;

    std::vector<std::unique_ptr<MidiDevice>> _midiInDevices;
};

