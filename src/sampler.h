// class to trigger samples from MIDI events

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "types.h"

class Sample;

class Sampler {

 public:

  Sampler();

  ~Sampler();

  // parse a YAML config string
  bool parseConfig(const std::string &config_string);

  // process a midi event at a particular velocity
  bool processEvent(midi_event_t event, midi_velocity velocity);

  // retreive audio samples
  std::shared_ptr<sample_vec> getAudio(nframes_t nframes);

 private:

  // load a sample from a file, and register it to be played on a
  // specified MIDI event
  bool addSample(midi_data_t data,
                 const std::string &sample_filename,
                 float pan);

  std::vector<Sample> samples;

  midi_sample_map sample_map;

  sample_vec audio_buffer;

};
