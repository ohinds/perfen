#include "keytracked_sampler.h"

using std::shared_ptr;

KeytrackedSampler::KeytrackedSampler(const std::string& sample_filename) {
  // TODO load audio
}

KeytrackedSampler::~KeytrackedSampler() {}

bool KeytrackedSampler::setRelativeRate(float new_relative_rate) {
  relative_rate = new_relative_rate;

  // TODO stretch some audio
}

shared_ptr<sample_vec> KeytrackedSampler::getAudio(
  nframes_t nframes, float position) const {

  // TODO use stretched audio

  shared_ptr<sample_vec> audio_frames(new sample_vec(nframes, 0));

  return audio_frames;
}
