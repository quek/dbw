#include "App.h"
#include "AudioEngineWindow.h"
#include "Composer.h"
#include "Command.h"
#include "Config.h"
#include "DropManager.h"
#include "command/AddTrack.h"

extern HWND gHwnd;

App::App() :
    _audioEngine(std::make_unique<AudioEngine>(this))
{
    Composer* composer = new Composer();
    composer->_commandManager.executeCommand(new command::AddTrack());
    addComposer(composer);

    _dropManager = new DropManager(this);
    RegisterDragDrop(gHwnd, _dropManager);
}

App::~App()
{
    if (_dropManager != nullptr)
    {
        RevokeDragDrop(gHwnd);
        delete _dropManager;
        _dropManager = nullptr;
    }
}

void App::render()
{
    if (_isDragging)
    {
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceExtern))	// we use an external source (i.e. not ImGui-created)
        {
            // replace "FILES" with whatever identifier you want - possibly dependant upon what type of files are being dragged
            // you can specify a payload here with parameter 2 and the sizeof(parameter) for parameter 3.
            // I store the payload within a vector of strings within the application itself so don't need it.
            ImGui::SetDragDropPayload(DDP_EXTERNAL_FILES, nullptr, 0);
            ImGui::BeginTooltip();
            ImGui::Text(getDropFiles()[0].c_str());
            ImGui::EndTooltip();
            ImGui::EndDragDropSource();
        }
    }

    if (gPreference.audioDeviceIndex == -1)
    {
        showAudioSetupWindow();
    }

    for (auto& x : _composers)
    {
        x->render();
    }
    if (_audioEngineWindow)
    {
        _audioEngineWindow->render();
    }
}

void App::addComposer(Composer* composer)
{
    composer->_app = this;
    _composers.emplace_back(composer);
}

void App::deleteComposer(Composer* composer)
{
    auto it = std::ranges::find_if(_composers, [composer](const auto& x) { return x.get() == composer; });
    if (it != _composers.end())
    {
        _composers.erase(it);
    }
}

void App::dragEnter(const std::vector<std::string>& files)
{
    _dropFiles = files;
    _isDragging = true;
}

void App::drop()
{
    _isDragging = false;
}

void App::runCommand()
{
    _requestAddComposers.clear();
    _requestDeleteComposers.clear();

    for (auto& composer : _composers)
    {
        composer->_commandManager.run();
    }

    if (_requestAddComposers.empty() && _requestDeleteComposers.empty())
    {
        return;
    }

    _audioEngine->stop();
    for (auto* composer : _requestDeleteComposers)
    {
        deleteComposer(composer);
    }
    for (auto* composer : _requestAddComposers)
    {
        addComposer(composer);
        composer->computeProcessOrder();
    }
    _audioEngine->start();
}

void App::requestAddComposer(Composer* composer)
{
    _requestAddComposers.push_back(composer);
}

void App::requestDeleteComposer(Composer* composer)
{
    _requestDeleteComposers.push_back(composer);
}

void App::showAudioSetupWindow()
{
    if (!_audioEngineWindow)
    {
        _audioEngineWindow = std::make_unique<AudioEngineWindow>(this);
    }
    _audioEngineWindow->show();
}

bool App::isDragging()
{
    return _isDragging;
}

void App::start()
{
    _audioEngine->start();
    _isStarted = true;
}

void App::stop()
{
    for (auto& x : _composers)
    {
        x->stop();
    }
    _audioEngine->stop();
    _isStarted = false;
}
