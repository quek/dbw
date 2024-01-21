#include "PluginEventList.h"

PluginEventList::PluginEventList() :
	_clap_input_events(clap_input_events{
		this,
		size,
		get,
		}),
		_clap_output_events(clap_output_events{
			this,
			try_push,
			})
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

clap_input_events_t* PluginEventList::clapInputEvents()
{
	return &_clap_input_events;
}

clap_output_events_t* PluginEventList::clapOutputEvents()
{
	return &_clap_output_events;
}
