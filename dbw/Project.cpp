#include "Project.h"
#include "tinyxml2/tinyxml2.h"
#include "Composer.h"
#include "util.h"

Project::Project(std::string name, Composer* composer) : _dir(projectDir()), _name(name), _composer(composer) {
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


    doc.SaveFile(projectXml().string().c_str());
}

std::filesystem::path Project::projectXml() {
    return _dir / _name /  "\\project.xml";
}
