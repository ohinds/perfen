#pragma once

#include <map>
#include <vector>

#include <jack/jack.h>
#include <jack/midiport.h>

typedef jack_default_audio_sample_t sample_t;

typedef std::vector<sample_t> sample_vec;

typedef jack_nframes_t nframes_t;

typedef jack_midi_event_t midi_event_t;

typedef jack_midi_data_t midi_data_t;

typedef unsigned char midi_velocity;

typedef std::map<midi_data_t, sample_vec> midi_sample_map;
