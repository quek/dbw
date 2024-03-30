#include "Connection.h"
#include "Config.h"
#include "Module.h"
#include "Track.h"

Connection::Connection(const nlohmann::json& json, SerializeContext& context) : Nameable(json, context)
{
    for (const auto& x : json.items())
    {
        std::string key = x.key();
        if (key == "_fromNekoRef")
        {
            _fromNekoRef = x.value();
        }
        else if (key == "_fromNekoRef")
        {
            _fromNekoRef = x.value();
        }
        else if (key == "_fromIndex")
        {
            _fromIndex = x.value();
        }
        else if (key == "_toNekoRef")
        {
            _toNekoRef = x.value();
        }
        else if (key == "_toIndex")
        {
            _toIndex = x.value();
        }
        else if (key == "_post")
        {
            _post = x.value();
        }
    }
}

Connection::Connection(Module* from, int fromIndex, Module* to, int toIndex, bool post) :
    _from(from), _fromIndex(fromIndex), _to(to), _toIndex(toIndex), _post(post)
{
    setLatency(0);

    if (bufferRequired())
    {
        _buffer = std::make_unique<AudioBuffer>();
        _buffer->ensure(gPreference.bufferSize, 2);
    }
    else
    {
        _buffer.reset();
    }
}

std::unique_ptr<AudioBuffer>& Connection::bufferGet()
{
    return _buffer;
}

void Connection::resolveModuleReference()
{
    _from = Neko::findByNekoId<Module>(_fromNekoRef);
    _to = Neko::findByNekoId<Module>(_toNekoRef);
}

void Connection::process(Module* to)
{
    if (_to != to || !_from->isStarting()) return;

    auto& from = _buffer ? *_buffer : (_post ? _from->_track->_processBuffer._out[_fromIndex] : _from->_track->_processBuffer._in[_fromIndex]);
    for (int channel = 0; channel < 2; ++channel)
    {
        bool xConstantp = from._constantp[channel];
        bool yConstantp = _to->_track->_processBuffer._in.at(_toIndex)._constantp[channel];
        auto& x = from.buffer32()[channel];
        auto& y = to->_track->_processBuffer._in.at(_toIndex).buffer32()[channel];
        auto y0 = y[0];
        for (size_t i = 0; i < gPreference.bufferSize; ++i)
        {
            size_t xi = i;
            if (xConstantp)
            {
                xi = 0;
            }
            if (_latency == 0)
            {
                if (yConstantp)
                {
                    y[i] = y0 + x[xi];
                }
                else
                {
                    y[i] += x[xi];
                }
            }
            else
            {
                auto& dcp = _dcpBuffer[channel];
                dcp.push_back(x[xi]);
                if (yConstantp)
                {
                    y[i] = y0 + dcp.front();
                }
                else
                {
                    y[i] += dcp.front();
                }
                dcp.pop_front();
            }
        }
        to->_track->_processBuffer._in.at(_toIndex)._constantp[channel] = false;
    }
}

std::string Connection::scLabel()
{
    auto label = std::format("{} {} {}", _from->_track->_name, _from->_name, _toIndex);
    if (!_post) label += " PRE";
    return label;
}

void Connection::setLatency(uint32_t latency)
{
    _latency = latency;
    if (_latency == 0)
    {
        return;
    }
    _dcpBuffer.clear();
    for (int i = 0; i < 2; ++i)
    {
        _dcpBuffer.emplace_back(latency, 0.0f);
    }
}

nlohmann::json Connection::toJson(SerializeContext& context)
{
    nlohmann::json json = Nameable::toJson(context);
    json["_fromIndex"] = _fromIndex;
    json["_toIndex"] = _toIndex;
    json["_fromNekoRef"] = _from->getNekoId();
    json["_toNekoRef"] = _to->getNekoId();
    json["_post"] = _post;
    return json;
}

bool Connection::bufferRequired()
{
    if (_from->_track == _to->_track)
    {
        auto modules = _from->_track->allModules();
        auto fromIt = std::ranges::find(modules, _from);
        auto toIt = std::ranges::find(modules, _to);
        if (std::distance(fromIt, toIt) >= 2)
        {
            return true;
        }
    }
    return false;
}
