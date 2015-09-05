// class to trigger samples from MIDI events

#pragma once

#include <memory>
#include <string>

#include "types.h"

class Sampler {

 public:

  Sampler();

  ~Sampler();

  // load a sample from a file, and register it to be played on a
  // specified MIDI event
  bool addSample(midi_data_t event, const std::string &sample_filename);

  // process a midi event at a particular velocity
  bool processEvent(midi_event_t event, midi_velocity velocity);

  // retreive audio samples
  std::shared_ptr<sample_vec> getAudio(nframes_t nframes);

 private:

  midi_sample_map sample_map;

  sample_vec audio_buffer;

};
