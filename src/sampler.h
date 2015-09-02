// class to trigger samples from MIDI events

#pragma once

#include <memory>
#include <string>

#include "types.h"

class Sampler {

 public:

  Sampler();

  ~Sampler();

  bool addSample(midi_event event, const sample_vec &sample);

  // process a midi event at a particular velocity, and return any
  // audio that may result from the event
  std::shared_ptr<sample_vec> processEvent(
    midi_event event, midi_velocity velocity);

 private:

  midi_sample_map sample_map;

};
