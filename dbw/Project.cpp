#include "Project.h"
#include <format>
#include "tinyxml2/tinyxml2.h"
#include "AudioEngine.h"
#include "Clip.h"
#include "ClipSlot.h"
#include "Composer.h"
#include "logger.h"
#include "Module.h"
#include "Note.h"
#include "PluginHost.h"
#include "PluginModule.h"
#include "Lane.h"
#include "util.h"

std::vector<std::unique_ptr<Note>> notesFromElement(tinyxml2::XMLElement* notesElement) {
    std::vector<std::unique_ptr<Note>> notes;

    for (auto noteElement = notesElement->FirstChildElement("Note");
         noteElement != nullptr;
         noteElement = noteElement->NextSiblingElement("Note")) {
        auto note = Note::fromXml(noteElement);
        notes.emplace_back(std::move(note));
    }
    return notes;
}

Project::Project(std::string name, Composer* composer) : _dir(::projectDir()), _name(name), _composer(composer) {
    std::filesystem::create_directories(_dir);
}

void Project::open(std::filesystem::path dir) {
    _idMap.clear();
    _idMapRev.clear();

    auto path = dir / "project.xml";
    auto parent_path = path.parent_path();
    _dir = parent_path.parent_path();
    _name = parent_path.filename();

    tinyxml2::XMLDocument doc;
    doc.LoadFile(path.string().c_str());

    _composer->stop();
    _composer->_audioEngine->stop();
    _composer->clear();

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
        auto channelElement = trackElement->FirstChildElement("Channel");
        std::string role = channelElement->Attribute("role");
        Track* track;
        if (role == "master") {
            MasterTrack* masterTrack = new MasterTrack(_composer);
            _composer->_masterTrack.reset(masterTrack);
            track = masterTrack;
        } else {
            track = new Track(name, _composer);
            _composer->_tracks.push_back(std::unique_ptr<Track>(track));
        }
        track->_trackLanes.clear();
        track->_modules.clear();
        setId(track, trackElement->Attribute("id"));

        for (auto deviceElement = channelElement->FirstChildElement("Devices")->FirstChildElement();
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
    for (auto lanesElement = lanesWrap->FirstChildElement("Lanes");
         lanesElement != nullptr;
         lanesElement = lanesElement->NextSiblingElement("Lanes")) {
        Track* track = (Track*)getObject(lanesElement->Attribute("track"));
        Lane* lane = new Lane();
        track->_trackLanes.emplace_back(lane);
        for (auto clipElement = lanesElement->FirstChildElement("Clips")->FirstChildElement("Clip");
             clipElement != nullptr;
             clipElement = clipElement->NextSiblingElement("Clip")) {
            lane->_clips.emplace_back(Clip::fromXml(clipElement));
        }
    }

    auto sceneMatrix = _composer->_sceneMatrix.get();
    for (auto sceneElement = doc.FirstChildElement("Project")->FirstChildElement("Scenes")->FirstChildElement("Scene");
         sceneElement != nullptr;
         sceneElement = sceneElement->NextSiblingElement()) {
        Scene* scene = new Scene(sceneMatrix);
        scene->_name = sceneElement->Attribute("name");
        sceneMatrix->_scenes.emplace_back(scene);
        auto lanesElement = sceneElement->FirstChildElement("Lanes");
        auto clipSlotElement = lanesElement->FirstChildElement("ClipSlot");
        for (auto& track : _composer->_tracks) {
            for (auto& lane : track->_trackLanes) {
                auto& clipSlot = scene->getClipSlot(lane.get());
                auto clipElement = clipSlotElement->FirstChildElement("Clip");
                if (clipElement != nullptr) {
                    clipSlot->_clip = Clip::fromXml(clipElement);
                }
                clipSlotElement = clipSlotElement->NextSiblingElement("ClipSlot");
            }
        }
    }

    _composer->_audioEngine->start();
    _isNew = false;
}

void Project::save() {
    _idMap.clear();
    _idMapRev.clear();

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
            writeTrack(doc, structure, track);
        }
        writeTrack(doc, structure, (Track*)(_composer->_masterTrack.get()), "master");
    }
    {
        auto* arrangement = project->InsertNewChildElement("Arrangement");
        {
            auto* lanesWrap = arrangement->InsertNewChildElement("Lanes");
            lanesWrap->SetAttribute("timeUnit", "beats");
            for (int trackIndex = 0; trackIndex < _composer->_tracks.size(); ++trackIndex) {
                Track* track = _composer->_tracks[trackIndex].get();
                for (auto& lane : track->_trackLanes) {
                    auto* lanes = lanesWrap->InsertNewChildElement("Lanes");
                    lanes->SetAttribute("track", getId(track).c_str());
                    auto* clipsElement = lanes->InsertNewChildElement("Clips");
                    for (auto& clip : lane->_clips) {
                        clipsElement->InsertEndChild(clip->toXml(&doc));
                    }
                }
            }
        }
    }
    {
        auto* scenesElement = project->InsertNewChildElement("Scenes");
        for (auto& scene : _composer->_sceneMatrix->_scenes) {
            auto* sceneElement = scenesElement->InsertNewChildElement("Scene");
            sceneElement->SetAttribute("name", scene->_name.c_str());
            auto lanesElement = sceneElement->InsertNewChildElement("Lanes");
            for (auto& track : _composer->_tracks) {
                for (auto& lane : track->_trackLanes) {
                    auto& clipSlot = scene->getClipSlot(lane.get());
                    auto clipSlotElement = lanesElement->InsertNewChildElement("ClipSlot");
                    if (clipSlot->_clip != nullptr) {
                        clipSlotElement->InsertEndChild(clipSlot->_clip->toXml(&doc));
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

    _isNew = false;
}

std::filesystem::path Project::projectDir() const {
    return _dir / _name;
}

std::filesystem::path Project::projectXml() const {
    return projectDir() / "project.xml";
}

void Project::writeTrack(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* structure, Track* track, const char* role) {
    auto* trackElement = structure->InsertNewChildElement("Track");
    trackElement->SetAttribute("id", generateId(track, "track").c_str());
    trackElement->SetAttribute("name", track->_name.c_str());
    trackElement->SetAttribute("contentType", "notes");
    trackElement->SetAttribute("loaded", true);
    {
        auto* channel = trackElement->InsertNewChildElement("Channel");
        {
            channel->SetAttribute("role", role);
            auto* devices = channel->InsertNewChildElement("Devices");
            for (auto module = track->_modules.begin(); module != track->_modules.end(); ++module) {
                devices->InsertEndChild((*module)->dawProject(&doc));
            }
        }
    }
}

bool Project::contaisId(void* x) {
    return _idMap.contains(x);
}

std::string Project::generateId(void* x, std::string prefix) {
    std::string id = prefix + std::to_string(_idMap.size());
    _idMap[x] = id;
    _idMapRev[id] = x;
    return id;
}

std::string Project::getId(void* x) {
    return _idMap[x];
}
void Project::setId(void* x, std::string id) {
    _idMap[x] = id;
    _idMapRev[id] = x;
}
void* Project::getObject(std::string id) {
    return _idMapRev[id];
}
