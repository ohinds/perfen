// Estimates the position in a keytrack by matching the keytrack with
// a live track

#pragma once

#include <string>

#include "types.h"

class KeyTracker {

 public:

  KeyTracker(const std::string& keytrack_file);

  virtual ~KeyTracker();

  bool init();

  bool start();

  bool addLiveAudio(const sample_vec& audio);

  // update the position and relative rate estimates based on the
  // audio added since the last call to this function
  bool update();

  // get the estimated position in the keytrack in units of proportion
  // [0,1]
  float getEstimatedPosition() const;

  // get the estimated relative rate of the live track relative to the
  // keytrack
  float getEstimatedRelativeRate() const;

  void setPeakThreshold(float threshold);

 private:

  std::string keytrack_file;

  sample_vec keytrack_audio;
  sample_vec unprocessed_audio;

  float estimated_position;
  float estimated_rate;

  // estimator parameters
  float peak_threshold;

};
