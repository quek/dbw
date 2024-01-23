#include "PluginEventList.h"

PluginEventList::PluginEventList() :
    _clap_input_events(clap_input_events{ this, size, get, }),
    _clap_output_events(clap_output_events{ this, try_push, })
{
}

// returns the number of events in the list
// uint32_t(CLAP_ABI* size)(const struct clap_input_events* list) {
uint32_t PluginEventList::size(const struct clap_input_events* list) {
    PluginEventList* self = (PluginEventList*)list->ctx;
    return (uint32_t)self->_events.size();
}

// Don't free the returned event, it belongs to the list
// const clap_event_header_t* (CLAP_ABI* get)(const struct clap_input_events* list, uint32_t index) {
const clap_event_header_t* PluginEventList::get(const struct clap_input_events* list, uint32_t index) {
    PluginEventList* self = (PluginEventList*)list->ctx;
    return self->_events[index];
}

// Pushes a copy of the event
// returns false if the event could not be pushed to the queue (out of memory?)
bool PluginEventList::try_push(const struct clap_output_events* list,
    const clap_event_header_t* event) {
    PluginEventList* self = (PluginEventList*)list->ctx;
    self->_events.push_back(event);
    return true;
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
    event->velocity = velocity / 127.0;
    _events.push_back(&event->header);
    _event_notes.push_back(std::unique_ptr<clap_event_note>(event));
}

void PluginEventList::noteOff(int16_t key, int16_t channel, double velocity, uint32_t sampleOffset) {
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
    event->velocity = velocity / 127.0;
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
    return &_clap_input_events;
}

clap_output_events_t* PluginEventList::clapOutputEvents()
{
    return &_clap_output_events;
}
