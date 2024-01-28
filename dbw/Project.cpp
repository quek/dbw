#include "Project.h"
#include <format>
#include "tinyxml2/tinyxml2.h"
#include "Column.h"
#include "Composer.h"
#include "Line.h"
#include "logger.h"
#include "Module.h"
#include "PluginHost.h"
#include "PluginModule.h"
#include "util.h"

Project::Project(std::string name, Composer* composer) : _dir(::projectDir()), _name(name), _composer(composer) {
    std::filesystem::create_directories(_dir);
}

void Project::open() {
    tinyxml2::XMLDocument doc;
    doc.LoadFile(projectXml().string().c_str());


    tinyxml2::XMLElement* tempo = doc.FirstChildElement("Project")->FirstChildElement("Transport")->FirstChildElement("Tempo");
    if (tempo != nullptr) {
        const char* bpmValue = tempo->Attribute("value");
        if (bpmValue) {
            _composer->_bpm = std::stof(bpmValue);
        }
    }
    for (auto* trackElement = doc.FirstChildElement("Project")->FirstChildElement("Structure")->FirstChildElement("Track");
         trackElement != nullptr;
         trackElement = trackElement->NextSiblingElement("Track")) {
        std::string name = trackElement->Attribute("name");
        Track* track = new Track(name, _composer);
        _composer->_tracks.push_back(std::unique_ptr<Track>(track));
        for (auto deviceElement = trackElement->FirstChildElement("Channel")->FirstChildElement("Devices")->FirstChildElement();
             deviceElement != nullptr;
             deviceElement = deviceElement->NextSiblingElement()) {
            if (deviceElement) {
                if (strcmp(deviceElement->Name(), "ClapPlugin") == 0) {
                    auto deviceId = deviceElement->Attribute("deviceID");
                    auto plugin = _composer->_pluginManager.findPlugin(deviceId);
                    if (plugin != nullptr) {
                        PluginHost* pluginHost = new PluginHost(track);
                        pluginHost->load((*plugin)["path"].get<std::string>(), (*plugin)["index"].get<uint32_t>());
                        Module* module = new PluginModule(pluginHost->_name, track, pluginHost);
                        track->_modules.push_back(std::unique_ptr<Module>(module));
                        auto state = deviceElement->FirstChildElement("State");
                        pluginHost->_statePath = state->Attribute("path");
                        pluginHost->loadState();
                    }
                }
            }
        }
    }
}

void Project::save() {
    tinyxml2::XMLDocument doc;
    auto* project = doc.NewElement("Project");
    doc.InsertEndChild(project);
    project->SetAttribute("version", "1.0");
    {
        auto* application = project->InsertNewChildElement("Application");
        application->SetAttribute("name", "DBetaW");
        application->SetAttribute("version", "0.0.1");
    }
    {
        auto* transport = project->InsertNewChildElement("Transport");
        {
            auto* tempo = transport->InsertNewChildElement("Tempo");
            tempo->SetAttribute("unit", "bpm");
            tempo->SetAttribute("value", _composer->_bpm);
        }
        {
            auto* timeSignature = transport->InsertNewChildElement("TimeSignature");
            timeSignature->SetAttribute("numerator", 4);
            timeSignature->SetAttribute("denominator", 4);
        }
    }
    {
        auto* structure = project->InsertNewChildElement("Structure");
        for (int i = 0; i < _composer->_tracks.size(); ++i) {
            Track* track = _composer->_tracks[i].get();
            auto* trackElement = structure->InsertNewChildElement("Track");
            trackElement->SetAttribute("id", std::format("track{}", i).c_str());
            trackElement->SetAttribute("name", track->_name.c_str());
            trackElement->SetAttribute("contentType", "notes");
            trackElement->SetAttribute("loaded", true);
            {
                auto* channel = trackElement->InsertNewChildElement("Channel");
                {
                    auto* devices = channel->InsertNewChildElement("Devices");
                    for (auto module = track->_modules.begin(); module != track->_modules.end(); ++module) {
                        devices->InsertEndChild((*module)->dawProject(&doc));
                    }
                }
            }
        }
    }
    {
        auto* arrangement = project->InsertNewChildElement("Arrangement");
        {
            auto* lanesWrap = arrangement->InsertNewChildElement("Lanes");
            lanesWrap->SetAttribute("timeUnit", "beats");
            for (int trackIndex = 0; trackIndex < _composer->_tracks.size(); ++trackIndex) {
                Track* track = _composer->_tracks[trackIndex].get();
                auto* lanes = arrangement->InsertNewChildElement("Lanes");
                lanes->SetAttribute("track", std::format("track{}", trackIndex).c_str());
                for (int columnIndex = 0; columnIndex < track->_ncolumns; ++columnIndex) {
                    auto* notes = lanes->InsertNewChildElement("Notes");
                    for (int lineIndex = 0; lineIndex < _composer->_maxLine; ++lineIndex) {
                        Column* column = track->_lines[lineIndex]->_columns[columnIndex].get();
                        if (!column->_note.empty()) {
                            auto* note = notes->InsertNewChildElement("Note");
                            // FIXME lpb と delay を計算に入れる
                            note->SetAttribute("time", lineIndex);
                            // FIXME 次のノートの開始位置から算出
                            note->SetAttribute("duration", column->_delay);
                            // FIXME MIDI key of this note.	あと OFF は入れない
                            note->SetAttribute("key", column->_note.c_str());
                            note->SetAttribute("vel", column->_velocity / 127.0);
                        }
                    }
                }
            }
        }
    }

    auto path = projectXml();
    std::filesystem::create_directories(path.parent_path());
    tinyxml2::XMLError error = doc.SaveFile(path.string().c_str());
    if (error != tinyxml2::XMLError::XML_SUCCESS) {
        // TODO
        logger->error("Save error {}", error);
    }
}

std::filesystem::path Project::projectDir() const {
    return _dir / _name;
}

std::filesystem::path Project::projectXml() const {
    return projectDir() / "project.xml";
}
