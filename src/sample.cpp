#include "sample.h"

#include <memory>
#include <string>

#include <sndfile.h>

#include "ui.h"
#include "util.h"

using std::copy;
using std::shared_ptr;
using std::string;

namespace {
  Ui ui(false);
}

Sample::Sample(const string &filename, float pan)
  : filename(filename)
  , pan(pan)
  , audio(0)
{}

bool Sample::load() {
  size_t num_channels = get_num_channels(filename);

  // mono
  if (num_channels == 1) {
    // TODO apply the pan
    shared_ptr<sample_vec> read_audio = load_mono_audio(filename);
    if (read_audio == NULL) {
      return false;
    }

    copy(read_audio->begin(), read_audio->end(), audio.left.begin());
    copy(read_audio->begin(), read_audio->end(), audio.right.begin());
  }
  else if(num_channels == 2) { // stereo
    shared_ptr<stereo_sample_vec> read_audio = load_stereo_audio(filename);
    if (read_audio == NULL) {
      return false;
    }

    audio = *read_audio;
  }
}

Sample::~Sample() {
}

bool Sample::getAudioFrames(nframes_t start_frame, nframes_t num_frames,
                            stereo_sample_vec *frames) const {

  if (start_frame >= getNumFrames()) {
    ui << "requested out of bounds frames";
    return false;
  }

  for (int channel = 0; channel < 2; channel++) {
    copy(audio[channel].begin() + start_frame,
         audio[channel].begin() + start_frame + num_frames,
         (*frames)[channel].begin());
  }

  return true;
}
