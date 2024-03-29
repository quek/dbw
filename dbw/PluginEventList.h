#pragma once
#include <clap/clap.h>
#include <vector>
#include <memory>
#include <set>
#include "pluginterfaces/vst/ivstevents.h"
#include "public.sdk/source/vst/hosting/eventlist.h"

class PluginEventList
{
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

    void push(const clap_event_header_t* event)
    {
        _events.push_back(event);
    }

    void noteAllOff(int16_t channel, uint32_t sampleOffset);
    void noteOn(int16_t key, int16_t channel, double velocity, uint32_t sampleOffset);
    void noteOff(int16_t key, int16_t channel, double velocity, uint32_t sampleOffset);
    void clear();

    clap_input_events_t* clapInputEvents();
    clap_output_events_t* clapOutputEvents();

    std::vector<std::unique_ptr<clap_event_note>>_event_notes;
    std::vector<std::unique_ptr<clap_event_midi>>_event_midis;

    Steinberg::Vst::EventList vst3InputEvents(std::set<int16_t>& noteOnKeys);
    Steinberg::Vst::EventList vst3OutputEvents();

private:
    std::vector<const clap_event_header_t* > _events;
    clap_input_events_t _clap_input_events;
    clap_output_events_t _clap_output_events;
};
