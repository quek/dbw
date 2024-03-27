#include "Module.h"
#include <mutex>
#include "imgui.h"
#include "AudioEngine.h"
#include "BuiltinModule.h"
#include "ClapModule.h"
#include "Composer.h"
#include "Command.h"
#include "Config.h"
#include "Fader.h"
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

bool Module::isWaitingForFrom()
{
    for (auto& connection : _connections)
    {
        if (connection->_to == this && !connection->_from->processedGet() && connection->_from->isStarting())
        {
            return true;
        }
    }
    return false;
}

bool Module::isWaitingForTo()
{
    for (auto& connection : _connections)
    {
        if (connection->_from == this && !connection->_to->processedGet() && connection->_to->isStarting())
        {
            return true;
        }
    }
    return false;
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
    _processed = true;
    return true;
}

void Module::prepare()
{
    _processed = false;
}

void Module::processConnections()
{
    for (auto& connection : _connections)
    {
        connection->process(this);
    }
}

bool Module::processedGet()
{
    return _processed;
}

void Module::processedSet(bool value)
{
    _processed = value;
}

void Module::connect(Module* from, int fromIndex, int toIndex, bool post)
{
    disconnect(nullptr, fromIndex, toIndex);
    _connections.emplace_back(new Connection(from, fromIndex, this, toIndex, post));
    from->_connections.emplace_back(new Connection(from, fromIndex, this, toIndex, post));
}

void Module::disconnect(Module* from, int /*fromIndex*/, int toIndex)
{
    auto it = std::ranges::find_if(_connections, [&](auto& x) { return x->_to == this && (!from || x->_from == from) && x->_toIndex == toIndex; });
    if (it == _connections.end()) return;
    _connections.erase(it);
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
