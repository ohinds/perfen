#include "keytracker.h"

KeyTracker::KeyTracker(const std::string& keytrack_file)
  : keytrack_file(keytrack_file)
  , estimated_position(0.)
  , estimated_rate(0.)
{}

KeyTracker::~KeyTracker() {}

bool KeyTracker::init() {
  return true;
}

bool KeyTracker::start() {
  return true;
}

bool KeyTracker::addLiveAudio(const sample_vec& audio) {
  return true;
}

// update the position and relative rate estimates based on the
// audio added since the last call to this function
bool KeyTracker::update() {
  return true;
}

// get the estimated position in the keytrack in units of proportion
// [0,1]
float KeyTracker::getEstimatedPosition() const {
  return estimated_position;
}

// get the estimated relative rate of the live track relative to the
// keytrack
float KeyTracker::getEstimatedRelativeRate() const {
  return estimated_rate;
}
