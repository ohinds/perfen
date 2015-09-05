// class to hold one audio sample

#pragma once

#include <cmath>
#include <memory>
#include <string>

#include "types.h"

class Sample {

 public:

  Sample(const std::string &filename, float pan);

  ~Sample();

  const std::string& getFilename() const {
    return filename;
  }

  nframes_t getNumFrames() const {
    return audio[0].size();
  }

  size_t getNumChannels() const {
    return audio.size();
  }

 private:

  std::string filename;

  std::vector<sample_vec> audio;
};
