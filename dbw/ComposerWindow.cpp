#include "ComposerWindow.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "AudioEngine.h"
#include "Composer.h"
#include "Command/OpenProject.h"
#include "Config.h"
#include "Error.h"
#include "ErrorWindow.h"
#include "GuiUtil.h"
#include "util.h"

ComposerWindow::ComposerWindow(Composer* composer) : _composer(composer)
{
}

void ComposerWindow::render()
{
    ImGui::SetNextWindowSizeConstraints(ImVec2(300, 200), ImVec2(FLT_MAX, FLT_MAX));

    // TODO imgui.ini 問題
    //auto composerWindowName = _composer->_project->_name.string() + "##Composer";
    ImGui::Begin("Composer", nullptr, ImGuiWindowFlags_NoScrollbar);

    bool playing = _composer->_playing;
    if (ToggleButton("Play", &playing))
    {
        if (playing)
        {
            _composer->play();
        }
        else
        {
            _composer->stop();
        }
    }
    ImGui::SameLine();
    ToggleButton("Loop", &_composer->_looping);
    ImGui::SameLine();
    ToggleButton("ScLk", &_composer->_scrollLock);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 5);
    ImGui::DragFloat("BPM", &_composer->_bpm, 1.0f, 0.0f, 999.0f, "%.02f");
    ImGui::SameLine();
    if (ImGui::Button("Undo"))
    {
        _composer->_commandManager.undo();
    }
    ImGui::SameLine();
    if (ImGui::Button("Redo"))
    {
        _composer->_commandManager.redo();
    }

    ImGui::SameLine();
    if (ImGui::Button("Open"))
    {
        _composer->_commandManager.executeCommand(new command::OpenProject());
    }

    ImGui::SameLine();
    if (ImGui::Button("Save"))
    {
        _composer->_project->save();
    }

    ImGui::SameLine();
    ImGui::Text("cpu %.02f", _composer->audioEngine()->_cpuLoad);

    ImGui::Text(_statusMessage.c_str());

    handleLocalShortcut();

    ImGui::End();

    handleGlobalShortcut();
}

void ComposerWindow::setStatusMessage(std::string message)
{
    _statusMessage = message;
}

void ComposerWindow::handleGlobalShortcut()
{
    // TODO テキスト入力中などは無効にしたい
    if (ImGui::IsKeyPressed(ImGuiKey_Space))
    {
        if (_composer->_playing)
        {
            _composer->stop();
        }
        else
        {
            _composer->play();
        }
        return;
    }

    auto& io = ImGui::GetIO();
    if (io.KeyCtrl)
    {
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Y)))
        {
            _composer->_commandManager.redo();
        }
        else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Z)))
        {
            _composer->_commandManager.undo();
        }
    }
}

void ComposerWindow::handleLocalShortcut()
{
    if (!canHandleInput())
    {
        return;
    }
}
