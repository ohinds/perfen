#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <map>
#include <vector>

#include <jack/jack.h>
#include <jack/midiport.h>

typedef jack_default_audio_sample_t sample_t;

typedef std::vector<sample_t> sample_vec;

typedef jack_nframes_t nframes_t;

typedef jack_midi_event_t midi_event_t;

typedef jack_midi_data_t midi_data_t;

typedef unsigned char midi_velocity;

// light container for stereo audio
class stereo_sample_vec {

public:

  sample_vec left;
  sample_vec right;

  stereo_sample_vec(nframes_t length)
    : left(length, 0)
    , right(length, 0)
    {};

  void resize(size_t new_length) {
    left.resize(new_length);
    right.resize(new_length);
  }

  void operator += (const stereo_sample_vec& other) {
    for (size_t channel = 0; channel < 2; channel++) {
      std::transform((*this)[channel].begin(),
                     (*this)[channel].end(),
                     other[channel].begin(),
                     (*this)[channel].begin(),
                     std::plus<sample_t>());
    }
  }

  sample_vec& operator [] (size_t channel) {
    if (channel == 0) {
      return left;
    }
    else if (channel == 1) {
      return right;
    }
    else {
      assert(false);
    }
  }

  const sample_vec& operator [] (size_t channel) const {
    if (channel == 0) {
      return left;
    }
    else if (channel == 1) {
      return right;
    }
    else {
      assert(false);
    }
  }
};
