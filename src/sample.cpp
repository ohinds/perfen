#include "sample.h"

#include <iostream>
#include <string>

#include <sndfile.h>

#include "ui.h"

using std::copy;
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
  SF_INFO info;
  SNDFILE *snd_file = sf_open(filename.c_str(), SFM_READ, &info);

  if (!snd_file) {
    ui << "Couldn't open " << filename << "\n";
    return false;
  }

  if ((info.format & 0xFF) != SF_FORMAT_PCM_16) {
    ui << "Unsupported audio format in " << filename << "\n";
    return false;
  }

  sample_vec read_audio(info.frames * info.channels);
  sf_count_t frames_read = sf_readf_float(snd_file, read_audio.data(),
                                          info.frames);
  sf_close(snd_file);

  if (frames_read != info.frames) {
    ui << "Error reading audio from " << filename << "\n";
    return false;
  }

  audio.resize(info.frames);

  // mono
  if (info.channels == 1) {
    // TODO apply the pan
    copy(read_audio.begin(), read_audio.end(), audio.left.begin());
    copy(read_audio.begin(), read_audio.end(), audio.right.begin());
  }
  else { // stereo
    copy(read_audio.begin(), read_audio.begin() + read_audio.size() / 2,
         audio.left.begin());
    copy(read_audio.begin() + read_audio. size() / 2, read_audio.end(),
         audio.right.begin());
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
