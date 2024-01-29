#include "Project.h"
#include <format>
#include "tinyxml2/tinyxml2.h"
#include "AudioEngine.h"
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
    auto path = getOpenFileName();
    if (path.empty()) {
        return;
    }
    auto parent_path = path.parent_path();
    _dir = parent_path.parent_path();
    _name = parent_path.filename();
    tinyxml2::XMLDocument doc;
    doc.LoadFile(path.string().c_str());

    _composer->stop();
    _composer->_audioEngine->stop();
    _composer->_tracks.clear();
    _composer->_commandManager.clear();

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
        Track* track;
        if (strcmp(trackElement->Attribute("id"), "MASTER") == 0) {
            MasterTrack* masterTrack = new MasterTrack(_composer);
            _composer->_masterTrack.reset(masterTrack);
            track = masterTrack;
        } else {
            track = new Track(name, _composer);
            _composer->_tracks.push_back(std::unique_ptr<Track>(track));
        }
        for (auto deviceElement = trackElement->FirstChildElement("Channel")->FirstChildElement("Devices")->FirstChildElement();
             deviceElement != nullptr;
             deviceElement = deviceElement->NextSiblingElement()) {
            if (deviceElement) {
                Module* module = _composer->_pluginManager.create(deviceElement, track);
                if (module != nullptr) {
                    track->_modules.push_back(std::unique_ptr<Module>(module));
                    module->start();
                }
            }
        }
    }
    auto* lanesWrap = doc.FirstChildElement("Project")->FirstChildElement("Arrangement")->FirstChildElement("Lanes");
    for (auto [lanes, track] = std::pair{ lanesWrap->FirstChildElement("Lanes"), _composer->_tracks.begin() };
         lanes != nullptr && track != _composer->_tracks.end();
         lanes = lanes->NextSiblingElement("Lanes"), ++track) {
        // FIXME see save
        bool first = true;
        for (auto* notes = lanes->FirstChildElement("Notes");
             notes != nullptr;
             notes = notes->NextSiblingElement("Notes")) {
            if (first) {
                first = false;
            } else {
                for (int i = 0; i < _composer->_maxLine; ++i) {
                    auto line = (*track)->_lines[i].get();
                    line->_columns.push_back(std::make_unique<Column>(line));
                }
                (*track)->_ncolumns++;
                (*track)->_lastKeys.push_back(0);
            }


            double lpb = _composer->_lpb;
            double endTime = std::numeric_limits<double>::max();
            for (auto note = notes->FirstChildElement("Note");
                 note != nullptr;
                 note = note->NextSiblingElement("Note")) {
                double time;
                note->QueryDoubleAttribute("time", &time);
                int lineIndex = static_cast<int>(time * 4);
                if (time > endTime) {
                    int endLineIndex = static_cast<int>(endTime * 4);
                    if (lineIndex > endLineIndex) {
                        auto& line = (*track)->_lines[endLineIndex];
                        auto& column = line->_columns.back();
                        column->_note = Midi::OFF;
                        double remainder = std::fmod(endTime, 1 / lpb);
                        unsigned char delay = static_cast<unsigned char>(remainder / (1 / lpb / 0x100));
                        column->_delay = delay;
                    }
                }

                int key;
                note->QueryIntAttribute("key", &key);
                double vel;
                note->QueryDoubleAttribute("vel", &vel);
                double remainder = std::fmod(time, 1 / lpb);
                unsigned char delay = static_cast<unsigned char>(remainder / (1 / lpb / 0x100));
                double duration;
                note->QueryDoubleAttribute("duration", &duration);
                endTime = time + duration;

                if ((*track)->_lines.size() <= lineIndex) {
                    continue;
                }
                auto& line = (*track)->_lines[lineIndex];
                auto& column = line->_columns.back();
                column->_note = numberToNote(static_cast<int16_t>(key));
                column->_velocity = static_cast<unsigned char>(vel * 127);
                column->_delay = delay;
            }
        }
    }

    _composer->_audioEngine->start();
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
            writeTrack(doc, structure, track, i);
        }
        writeTrack(doc, structure, (Track*)(_composer->_masterTrack.get()), "MASTER");
    }
    {
        auto* arrangement = project->InsertNewChildElement("Arrangement");
        {
            auto* lanesWrap = arrangement->InsertNewChildElement("Lanes");
            lanesWrap->SetAttribute("timeUnit", "beats");
            for (int trackIndex = 0; trackIndex < _composer->_tracks.size(); ++trackIndex) {
                Track* track = _composer->_tracks[trackIndex].get();
                auto* lanes = lanesWrap->InsertNewChildElement("Lanes");
                lanes->SetAttribute("track", std::format("track{}", trackIndex).c_str());
                for (int columnIndex = 0; columnIndex < track->_ncolumns; ++columnIndex) {
                    auto* notes = lanes->InsertNewChildElement("Notes");

                    auto lastKey = NOTE_NONE;
                    double startTime = 0.0;
                    double velocity = 0.0;
                    double delay = 0;
                    double lpb = _composer->_lpb;
                    for (int lineIndex = 0; lineIndex < _composer->_maxLine; ++lineIndex) {
                        Column* column = track->_lines[lineIndex]->_columns[columnIndex].get();
                        int16_t key = noteToNumber(column->_note);
                        if (key == NOTE_NONE) {
                            continue;
                        }
                        double time = lineIndex / lpb + column->_delay / (lpb * 256.0);
                        if (lastKey != NOTE_NONE) {
                            auto* note = notes->InsertNewChildElement("Note");
                            note->SetAttribute("key", lastKey);
                            note->SetAttribute("time", startTime);
                            note->SetAttribute("duration", time - startTime);
                            note->SetAttribute("vel", velocity);
                            startTime = time;
                        }
                        if (key == NOTE_OFF) {
                            lastKey = NOTE_NONE;
                        } else {
                            lastKey = key;
                            startTime = time;
                            velocity = column->_velocity / 127.0;
                            delay = column->_delay;
                        }
                    }
                    if (lastKey != NOTE_NONE) {
                        auto* note = notes->InsertNewChildElement("Note");
                        note->SetAttribute("key", lastKey);
                        note->SetAttribute("time", startTime);
                        note->SetAttribute("duration", 1 / lpb - (delay / (lpb * 256.0)));
                        note->SetAttribute("vel", velocity);
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
        logger->error("Save error!");
    }
}

std::filesystem::path Project::projectDir() const {
    return _dir / _name;
}

std::filesystem::path Project::projectXml() const {
    return projectDir() / "project.xml";
}

void Project::writeTrack(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* structure, Track* track, int index) {
    writeTrack(doc, structure, track, std::format("track{}", index).c_str());
}

void Project::writeTrack(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* structure, Track* track, const char* id) {
    auto* trackElement = structure->InsertNewChildElement("Track");
    trackElement->SetAttribute("id", id);
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
