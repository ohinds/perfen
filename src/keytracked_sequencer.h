// class to play out a sequence of samples along with a keytrack

#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "sample.h"
#include "types.h"

typedef std::pair<float, Sample> positioned_sample;
typedef std::vector<positioned_sample> sample_sequence;

class KeytrackedSequencer {

 public:

  KeytrackedSequencer();

  ~KeytrackedSequencer();

  // parse a YAML config string
  bool parseConfig(const std::string &config_string);

  bool setPosition(float new_position);

  bool setRelativeRate(float new_relative_rate);

  // get the specified number of audio samples at the current position
  // and relative rate
  std::shared_ptr<stereo_sample_vec> getAudio(nframes_t nframes) const;

 private:

  bool addPositionedSample(
    float position, const std::string &filename, float pan);

  sample_sequence sequence;

  float position;
  float relative_rate;
};
