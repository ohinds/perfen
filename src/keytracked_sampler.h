// class to play a sample that's aligned to a keytrack

#pragma once

#include <memory>
#include <string>

#include "types.h"

class KeytrackedSampler {

 public:

  KeytrackedSampler(const std::string& sample_filename);

  ~KeytrackedSampler();

  bool setRelativeRate(float new_relative_rate);

  // get the specified number of audio samples at the specified
  // position in the keytrack, at the current relative rate
  std::shared_ptr<sample_vec> getAudio(nframes_t nframes, float position) const;

 private:

  sample_vec audio;
  sample_vec stretched_audio;

  float relative_rate;
};
