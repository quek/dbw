#include "Module.h"
#include <mutex>
#include "imgui.h"
#include "AudioEngine.h"
#include "BuiltinModule.h"
#include "ClapModule.h"
#include "Composer.h"
#include "Command.h"
#include "Config.h"
#include "Track.h"
#include "Vst3Module.h"

Module::Module(const nlohmann::json& json, SerializeContext& context) : Nameable(json, context)
{
    for (const auto& jsn : json["_connections"])
    {
        _connections.emplace_back(new Connection(jsn, context));
    }
}

Module::~Module()
{
    closeGui();
    stop();
}

void Module::start()
{
    _track->_processBuffer.ensure(gPreference.bufferSize, max(_ninputs, _noutputs), 2);
    _isStarting = true;
}

void Module::render(std::vector<Module*>& selectedModules, float width, float height)
{
    ImGui::PushID(this);
    ImGuiChildFlags childFlags = ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY;
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None;

    bool selected = std::ranges::find(selectedModules, this) != selectedModules.end();
    if (selected)
    {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0x40, 0x40, 0x40, 0x80));
    }
     
    if (ImGui::BeginChild("##module", ImVec2(width, height), childFlags, windowFlags))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32_BLACK_TRANS);
        if (ImGui::Button(_collapsed ? "▶" : "▼"))
        {
            _collapsed = !_collapsed;
        }

        ImGui::SameLine();
        if (ImGui::Button(_name.c_str()))
        {
            if (_didOpenGui) closeGui(); else openGui();
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();

        if (!_collapsed)
        {
            if (ImGui::IsItemHovered() && !_connections.empty())
            {
                std::ostringstream ost;
                for (auto& c : _connections)
                {
                    if (c->_to == this)
                    {
                        ost << std::format("{} {} {} ⇒ {}", c->_from->_track->_name, c->_from->_name, c->_fromIndex, c->_toIndex)
                            << std::endl;
                    }
                    else
                    {
                        ost << std::format("{} ⇒ {} {} {}", c->_fromIndex, c->_to->_track->_name, c->_to->_name, c->_toIndex) << std::endl;
                    }
                }
                auto str = ost.str();
                if (!str.empty())
                {
                    ImGui::SetTooltip(str.c_str());
                }
            }

            if (_ninputs > 1)
            {
                if (ImGui::Button("Sidechain"))
                {
                    // TODO inputIndex
                    _track->getComposer()->_sideChainInputSelector->open(this, 1);
                }
            }

            renderContent();
        }

        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            selectedModules.clear();
            selectedModules.push_back(this);
        }
    }
    ImGui::EndChild();
    if (selected) ImGui::PopStyleColor();
    ImGui::PopID();
}

bool Module::process(ProcessBuffer* /*buffer*/, int64_t /*steadyTime*/)
{
    return true;
}

void Module::processConnections()
{
    for (auto& connection : _connections)
    {
        connection->process(this);
    }
}

void Module::connect(Module* from, int outputIndex, int inputIndex)
{
    _connections.emplace_back(new Connection(from, outputIndex, this, inputIndex));
    from->_connections.emplace_back(new Connection(from, outputIndex, this, inputIndex));
}

int Module::nbuses() const
{
    return max(_ninputs, _noutputs);
}

ProcessBuffer& Module::getProcessBuffer()
{
    return _track->_processBuffer;
}

uint32_t Module::getComputedLatency()
{
    return _computedLatency;
}

void Module::setComputedLatency(uint32_t computedLatency)
{
    _computedLatency = computedLatency;
    for (auto& connection : _connections)
    {
        if (connection->_to == this)
        {
            if (connection->_from->getComputedLatency() < computedLatency)
            {
                connection->setLatency(computedLatency - connection->_from->getComputedLatency());
            }
            else
            {
                connection->setLatency(0);
            }
        }
    }
}

std::unique_ptr<Param>& Module::getParam(uint32_t paramId)
{
    return _idParamMap[paramId];
}

void Module::addParameterChange(Param* param, int32_t sampleOffset)
{
    addParameterChange(param, param->getValue(), sampleOffset);
}

void Module::updateEditedParamIdList(ParamId id)
{
    if (!getParam(id)->canAutomate())
    {
        return;
    }

    auto it = std::ranges::find(_editedParamIdList, id);
    if (it != _editedParamIdList.end())
    {
        _editedParamIdList.erase(it);
    }
    _editedParamIdList.push_front(id);
}

Module* Module::create(std::string& type, std::string& id)
{
    if (type == "builtin")
    {
        return BuiltinModule::create(id);
    }
    else if (type == "vst3")
    {
        return Vst3Module::create(id);
    }
    else if (type == "clap")
    {
        //return ClapModule::create(id);
    }
    return nullptr;
}

Module* Module::fromJson(const nlohmann::json& json, SerializeContext& context)
{
    auto& type = json["type"];
    if (type == "builtin")
    {
        //return BuiltinModule::fromJson(json, context);
    }
    else if (type == "vst3")
    {
        return new Vst3Module(json, context);
    }
    else if (type == "clap")
    {
        //return PluginModule::fromJson(json, context);
    }
    return nullptr;
}

nlohmann::json Module::toJson(SerializeContext& context)
{
    nlohmann::json json = Nameable::toJson(context);

    nlohmann::json connections = nlohmann::json::array();
    for (const auto& connection : _connections)
    {
        connections.emplace_back(connection->toJson(context));
    }
    json["_connections"] = connections;

    return json;
}
