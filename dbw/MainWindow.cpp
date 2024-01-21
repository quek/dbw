#include "MainWindow.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "PluginHost.h"

MainWindow::MainWindow(AudioEngine* audioEngine) : _audioEngine(audioEngine)
{
}

void MainWindow::render()
{
	ImGui::Begin("main window");   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
	ImGui::Text("Plugin");
	ImGui::InputText("plugin path", &_pluginPath);
	if (ImGui::Button("Load")) {
		_pluginHost = _audioEngine->addPlugin(_pluginPath);
	}
	if (ImGui::Button("Open")) {
		if (_pluginHost) {
			_pluginHost->openGui();
		}
	}
	if (ImGui::Button("Close")) {
		if (_pluginHost) {
			_pluginHost->closeGui();
		}
	}
	ImGui::End();
}
