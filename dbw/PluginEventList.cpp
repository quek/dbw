#include "PluginEventList.h"

PluginEventList::PluginEventList() :
    _clap_input_events(clap_input_events{ this, size, get, }),
    _clap_output_events(clap_output_events{ this, try_push, })
{
}

// returns the number of events in the list
// uint32_t(CLAP_ABI* size)(const struct clap_input_events* list) {
uint32_t PluginEventList::size(const struct clap_input_events* list)
{
    PluginEventList* self = (PluginEventList*)list->ctx;
    return static_cast<uint32_t>(self->_events.size());
}

// Don't free the returned event, it belongs to the list
// const clap_event_header_t* (CLAP_ABI* get)(const struct clap_input_events* list, uint32_t index) {
const clap_event_header_t* PluginEventList::get(const struct clap_input_events* list, uint32_t index)
{
    PluginEventList* self = (PluginEventList*)list->ctx;
    return self->_events[index];
}

// Pushes a copy of the event
// returns false if the event could not be pushed to the queue (out of memory?)
bool PluginEventList::try_push(const struct clap_output_events* list,
                               const clap_event_header_t* event)
{
    PluginEventList* self = (PluginEventList*)list->ctx;
    self->_events.push_back(event);
    return true;
}

void PluginEventList::noteAllOff(int16_t channel, uint32_t sampleOffset)
{
    clap_event_midi* event = new clap_event_midi;
    event->header.space_id = CLAP_CORE_EVENT_SPACE_ID;
    event->header.type = CLAP_EVENT_MIDI;
    event->header.time = sampleOffset;
    event->header.flags = 0;
    event->header.size = sizeof(clap_event_midi);
    event->data[0] = 0xB0 | (channel & 0xF); // MIDIチャンネル設定
    event->data[1] = 0x7B; // CC番号123
    event->data[2] = 0x00; // CCの値
    _events.push_back(&event->header);
    _event_midis.push_back(std::unique_ptr<clap_event_midi>(event));
}

void PluginEventList::noteOn(int16_t key, int16_t channel, double velocity, uint32_t sampleOffset)
{
    clap_event_note* event = new clap_event_note;
    event->header.space_id = CLAP_CORE_EVENT_SPACE_ID;
    event->header.type = CLAP_EVENT_NOTE_ON;
    event->header.time = sampleOffset;
    event->header.flags = 0;
    event->header.size = sizeof(clap_event_note);
    event->port_index = 0;
    event->key = key;
    event->channel = channel;
    event->note_id = -1;
    event->velocity = velocity;
    _events.push_back(&event->header);
    _event_notes.push_back(std::unique_ptr<clap_event_note>(event));
}

void PluginEventList::noteOff(int16_t key, int16_t channel, double velocity, uint32_t sampleOffset)
{
    clap_event_note* event = new clap_event_note;
    event->header.space_id = CLAP_CORE_EVENT_SPACE_ID;
    event->header.type = CLAP_EVENT_NOTE_OFF;
    event->header.time = sampleOffset;
    event->header.flags = 0;
    event->header.size = sizeof(clap_event_note);
    event->port_index = 0;
    event->key = key;
    event->channel = channel;
    event->note_id = -1;
    event->velocity = velocity;
    _events.push_back(&event->header);
    _event_notes.push_back(std::unique_ptr<clap_event_note>(event));
}

void PluginEventList::clear()
{
    _events.clear();
    _event_notes.clear();
}

clap_input_events_t* PluginEventList::clapInputEvents()
{
    // ProcessBuffer::swapInOut() で this が書き換えられるの再設定する
    _clap_input_events.ctx = this;
    return &_clap_input_events;
}

clap_output_events_t* PluginEventList::clapOutputEvents()
{
    // ProcessBuffer::swapInOut() で this が書き換えられるの再設定する
    _clap_input_events.ctx = this;
    return &_clap_output_events;
}

Steinberg::Vst::EventList PluginEventList::vst3InputEvents(std::set<int16_t>& noteOnKeys)
{
    Steinberg::Vst::EventList _vstInputEvents;
    for (auto& x : _events)
    {
        Steinberg::Vst::Event event{};
        event.sampleOffset = x->time;
        event.ppqPosition = 0;
        event.flags = Steinberg::Vst::Event::kIsLive;
        if (x->type == CLAP_EVENT_NOTE_ON)
        {
            clap_event_note* eventNote = (clap_event_note*)x;
            event.type = Steinberg::Vst::Event::kNoteOnEvent;
            event.noteOn.channel = eventNote->channel;
            event.noteOn.pitch = eventNote->key;
            event.noteOn.velocity = eventNote->velocity;
            event.noteOn.length = 0;
            event.noteOn.tuning = 0;
            event.noteOn.noteId = -1;
            noteOnKeys.insert(eventNote->key);
            _vstInputEvents.addEvent(event);
        }
        else if (x->type == CLAP_EVENT_NOTE_OFF)
        {
            clap_event_note* eventNote = (clap_event_note*)x;
            event.type = Steinberg::Vst::Event::kNoteOffEvent;
            event.noteOff.channel = eventNote->channel;
            event.noteOff.pitch = eventNote->key;
            event.noteOff.velocity = eventNote->velocity;
            event.noteOff.tuning = 0;
            event.noteOff.noteId = -1;
            noteOnKeys.erase(eventNote->key);
            _vstInputEvents.addEvent(event);
        }
        else if (x->type == CLAP_EVENT_MIDI)
        {
            clap_event_midi* eventMidi = (clap_event_midi*)x;
            // All Notes Off
            if (eventMidi->data[1] == 0x7B)
            {
                event.type = Steinberg::Vst::Event::kNoteOffEvent;
                event.noteOff.channel = eventMidi->data[0] & 0x0f;
                event.noteOff.velocity = 1.0f;
                event.noteOff.tuning = 0;
                event.noteOff.noteId = -1;
                for (auto& key : noteOnKeys)
                {
                    event.noteOff.pitch = key;
                    _vstInputEvents.addEvent(event);
                }
                noteOnKeys.clear();
            }
        }
    }
    return _vstInputEvents;
}

Steinberg::Vst::EventList PluginEventList::vst3OutputEvents()
{
    Steinberg::Vst::EventList _vstOutputEvents;
    _vstOutputEvents.clear();
    return _vstOutputEvents;
}
