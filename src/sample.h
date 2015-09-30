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

  // loads the audio from the specified file
  bool load();

  const std::string& getFilename() const {
    return filename;
  }

  nframes_t getNumFrames() const {
    return audio[0].size();
  }

  bool getAudioFrames(nframes_t start_frame, nframes_t num_frames,
                      stereo_sample_vec* samples) const;

 private:

  std::string filename;
  float pan;

  stereo_sample_vec audio;
};
