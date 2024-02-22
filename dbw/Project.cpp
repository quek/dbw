#include "Project.h"
#include <format>
#include "tinyxml2/tinyxml2.h"
#include "AudioEngine.h"
#include "Clip.h"
#include "ClipSlot.h"
#include "Composer.h"
#include "Connection.h"
#include "logger.h"
#include "Module.h"
#include "Note.h"
#include "ClapHost.h"
#include "ClapModule.h"
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
    _composer->_masterTrack = nullptr;

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
        auto track = Track::fromXml(trackElement, _composer);
        track->_lanes.clear();
        uint64_t id = 0;
        trackElement->QueryUnsigned64Attribute("id", &id);
        if (id != 0) {
            track->setNekoId(id);
        }
        if (_composer->_masterTrack) {
            _composer->_tracks.emplace_back(std::move(track));
        } else {
            _composer->_masterTrack = std::move(track);
        }
    }

    for (auto& track : _composer->_tracks) {
        for (auto& module : track->_modules) {
            for (auto& connection : module->_connections) {
                connection->resolveModuleReference();
            }
        }
    }

    
    auto* lanesWrap = doc.FirstChildElement("Project")->FirstChildElement("Arrangement")->FirstChildElement("Lanes");
    for (auto lanesElement = lanesWrap->FirstChildElement("Lanes");
         lanesElement != nullptr;
         lanesElement = lanesElement->NextSiblingElement("Lanes")) {
        uint64_t id = 0;
        lanesElement->QueryUnsigned64Attribute("track", &id);
        Track* track = Neko::findByNekoId<Track>(id);
        Lane* lane = new Lane();
        track->_lanes.emplace_back(lane);
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
        {
            for (auto& lane : _composer->_masterTrack->_lanes) {
                auto& clipSlot = scene->getClipSlot(lane.get());
                auto clipElement = clipSlotElement->FirstChildElement("Clip");
                if (clipElement != nullptr) {
                    clipSlot->_clip = Clip::fromXml(clipElement);
                }
                clipSlotElement = clipSlotElement->NextSiblingElement("ClipSlot");
            }
        }
        for (auto& track : _composer->_tracks) {
            for (auto& lane : track->_lanes) {
                auto& clipSlot = scene->getClipSlot(lane.get());
                auto clipElement = clipSlotElement->FirstChildElement("Clip");
                if (clipElement != nullptr) {
                    clipSlot->_clip = Clip::fromXml(clipElement);
                }
                clipSlotElement = clipSlotElement->NextSiblingElement("ClipSlot");
            }
        }
    }

    _composer->computeProcessOrder();
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
        structure->InsertEndChild(_composer->_masterTrack->toXml(&doc));
        for (auto& track : _composer->_tracks) {
            structure->InsertEndChild(track->toXml(&doc));
        }
    }
    {
        auto* arrangement = project->InsertNewChildElement("Arrangement");
        {
            auto* lanesWrap = arrangement->InsertNewChildElement("Lanes");
            lanesWrap->SetAttribute("timeUnit", "beats");
            {
                for (auto& lane : _composer->_masterTrack->_lanes) {
                    auto* lanes = lanesWrap->InsertNewChildElement("Lanes");
                    lanes->SetAttribute("track", _composer->_masterTrack->nekoId());
                    auto* clipsElement = lanes->InsertNewChildElement("Clips");
                    for (auto& clip : lane->_clips) {
                        clipsElement->InsertEndChild(clip->toXml(&doc));
                    }
                }
            }
            for (auto& track : _composer->_tracks) {
                for (auto& lane : track->_lanes) {
                    auto* lanes = lanesWrap->InsertNewChildElement("Lanes");
                    lanes->SetAttribute("track", track->nekoId());
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
            {
                for (auto& lane : _composer->_masterTrack->_lanes) {
                    auto& clipSlot = scene->getClipSlot(lane.get());
                    auto clipSlotElement = lanesElement->InsertNewChildElement("ClipSlot");
                    if (clipSlot->_clip != nullptr) {
                        clipSlotElement->InsertEndChild(clipSlot->_clip->toXml(&doc));
                    }
                }
            }
            for (auto& track : _composer->_tracks) {
                for (auto& lane : track->_lanes) {
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

