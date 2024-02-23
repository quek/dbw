#include "AudioEngineWindow.h"
#include <format>
#include <sstream>
#include <string>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "AudioEngine.h"
#include "Composer.h"
#include "Config.h"

static std::vector<double> PrintSupportedStandardSampleRates(
    const PaStreamParameters* inputParameters,
    const PaStreamParameters* outputParameters) {
    std::vector<double> supportedStandardSampleRates;
    static double standardSampleRates[] = {
        44100.0, 48000.0, 88200.0, 96000.0, 192000.0, -1 /* negative terminated  list */
    };
    int     i, printCount;
    PaError err;

    printCount = 0;
    for (i = 0; standardSampleRates[i] > 0; i++) {
        err = Pa_IsFormatSupported(inputParameters, outputParameters, standardSampleRates[i]);
        if (err == paFormatIsSupported) {
            supportedStandardSampleRates.push_back(standardSampleRates[i]);
        }
    }
    return  supportedStandardSampleRates;
}

AudioEngineWindow::AudioEngineWindow(Composer* composer) : _composer(composer) {
    _deviceIndex = gPreference.audioDeviceIndex;
    auto apiCount = Pa_GetHostApiCount();
    for (auto i = 0; i < apiCount; ++i) {
        auto apiInfo = Pa_GetHostApiInfo(i);
        _apiInfos.push_back(apiInfo);
    }

    auto deviceCount = Pa_GetDeviceCount();
    for (auto i = 0; i < deviceCount; ++i) {
        auto deviceInfo = Pa_GetDeviceInfo(i);
        _deviceInfos.push_back(deviceInfo);

        PaStreamParameters inputParameters{}, outputParameters{};
        inputParameters.device = i;
        inputParameters.channelCount = deviceInfo->maxInputChannels;
        inputParameters.sampleFormat = paFloat32;
        inputParameters.suggestedLatency = 0; /* ignored by Pa_IsFormatSupported() */
        inputParameters.hostApiSpecificStreamInfo = nullptr;

        outputParameters.device = i;
        outputParameters.channelCount = deviceInfo->maxOutputChannels;
        outputParameters.sampleFormat = paFloat32;
        outputParameters.suggestedLatency = 0; /* ignored by Pa_IsFormatSupported() */
        outputParameters.hostApiSpecificStreamInfo = nullptr;

        if (inputParameters.channelCount > 0 && outputParameters.channelCount > 0) {
            _supportedStandardSampleRates.push_back(PrintSupportedStandardSampleRates(&inputParameters, &outputParameters));
        } else if (inputParameters.channelCount > 0) {
            _supportedStandardSampleRates.push_back(PrintSupportedStandardSampleRates(&inputParameters, nullptr));
        } else {
            _supportedStandardSampleRates.push_back(PrintSupportedStandardSampleRates(nullptr, &outputParameters));
        }
    }
}

void AudioEngineWindow::render() {
    ImGui::OpenPopup("Audio Engine");
    if (ImGui::BeginPopupModal("Audio Engine", &_composer->_composerWindow->_showAudioEngineWindow)) {
        auto apiInfoIndex = -1;
        for (auto& apiInfo : _apiInfos) {
            ++apiInfoIndex;
            if (strcmp("ASIO", apiInfo->name) != 0) {
                continue;
            }
            if (ImGui::TreeNode(apiInfo->name)) {
                auto deviceIndex = -1;
                for (auto& deviceInfo : _deviceInfos) {
                    ++deviceIndex;
                    if (deviceInfo->hostApi == apiInfoIndex) {
                        if (ImGui::Selectable(deviceInfo->name, _deviceIndex == deviceIndex)) {
                            _deviceIndex = deviceIndex;
                        }
                        std::stringstream s1;
                        s1 << "    in " << deviceInfo->maxInputChannels
                            << " out " << deviceInfo->maxOutputChannels
                            << "\n    defaultLowInputLatency " << deviceInfo->defaultLowInputLatency
                            << " defaultLowOutputLatency " << deviceInfo->defaultLowOutputLatency
                            << " defaultHighInputLatency " << deviceInfo->defaultHighInputLatency
                            << "\n    defaultHighOutputLatency " << deviceInfo->defaultHighOutputLatency
                            << " defaultSampleRate " << deviceInfo->defaultSampleRate;
                        ImGui::Text(s1.str().c_str());
                        std::stringstream s2;
                        s2 << "    Sample Rate";
                        for (auto rate : _supportedStandardSampleRates[deviceIndex]) {
                            s2 << " " << rate;
                        }
                        ImGui::Text(s2.str().c_str());
                    }
                }
                ImGui::TreePop();
            }
        }
        ImGui::Separator();
        if (_deviceIndex != -1) {
            ImGui::Text(std::format("【{}】{}", _apiInfos[_deviceInfos[_deviceIndex]->hostApi]->name, _deviceInfos[_deviceIndex]->name).c_str());
        } else {
            ImGui::Text("--");
        }
        ImGui::SetNextItemWidth(ImGui::GetFontSize() * 7);
        ImGui::InputDouble("Sample Rate", &_sampleRate);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetFontSize() * 7);
        ImGui::InputScalar("Buffer Size", ImGuiDataType_U32, &_bufferSize);
        ImGui::BeginDisabled(_composer->audioEngine()->_isStarted);
        if (ImGui::Button("Apply")) {
            gPreference.audioDeviceIndex = _deviceIndex;
            gPreference.sampleRate = _sampleRate;
            gPreference.bufferSize = _bufferSize;
            gPreference.save();
            _composer->audioEngine()->start();
            _composer->_composerWindow->_showAudioEngineWindow = false;
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        if (ImGui::Button("Start Audio Engign")) {
            _composer->audioEngine()->start();
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop Audio Engign")) {
            _composer->stop();
            _composer->audioEngine()->stop();
        }

        ImGui::EndPopup();
    }

}
