#pragma once
#include <clap/clap.h>
#include <vector>

class PluginEventList {
public:
	PluginEventList();
	// returns the number of events in the list
	// uint32_t(CLAP_ABI* size)(const struct clap_input_events* list) {
	static uint32_t size(const struct clap_input_events* list);

	// Don't free the returned event, it belongs to the list
	// const clap_event_header_t* (CLAP_ABI* get)(const struct clap_input_events* list, uint32_t index) {
	static const clap_event_header_t* get(const struct clap_input_events* list, uint32_t index);

	// Pushes a copy of the event
	// returns false if the event could not be pushed to the queue (out of memory?)
	static bool try_push(const struct clap_output_events* list, const clap_event_header_t* event);

	clap_input_events_t* clapInputEvents();
	clap_output_events_t* clapOutputEvents();
private:
	std::vector<const clap_event_header_t* > _events;
	clap_input_events_t _clap_input_events;
	clap_output_events_t _clap_output_events;
};
