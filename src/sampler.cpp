#include "sampler.h"

using std::shared_ptr;
using std::string;

Sampler::Sampler()
{}

Sampler::~Sampler()
{}


bool Sampler::addSample(midi_data_t data, const string &sample_filename) {
  if (sample_map.find(data) != sample_map.end()) {
    // TODO print an error message
    return false;
  }

  // TODO load audio file

  return true;
}

bool Sampler::processEvent(midi_event_t event, midi_velocity velocity) {

  return true;
}

shared_ptr<sample_vec> Sampler::getAudio(nframes_t nframes) {

  shared_ptr<sample_vec> result_audio(new sample_vec(nframes, 0));

  return result_audio;
}
