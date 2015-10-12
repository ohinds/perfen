#include "util.h"

#include <sndfile.h>

#include "ui.h"

using std::copy;
using std::shared_ptr;
using std::string;

namespace {
Ui ui(false);
}

size_t get_num_channels(const std::string& filename) {
  SF_INFO info;
  SNDFILE *snd_file = sf_open(filename.c_str(), SFM_READ, &info);

  if (!snd_file) {
    ui << "Couldn't open " << filename << "\n";
    return 0;
  }

  if ((info.format & 0xFF) != SF_FORMAT_PCM_16) {
    ui << "Unsupported audio format in " << filename << "\n";
    return 0;
  }

  return info.channels;
}

shared_ptr<sample_vec> load_mono_audio(const std::string &filename) {
  SF_INFO info;
  SNDFILE *snd_file = sf_open(filename.c_str(), SFM_READ, &info);

  if (!snd_file) {
    ui << "Couldn't open " << filename << "\n";
    return shared_ptr<sample_vec>(NULL);
  }

  if ((info.format & 0xFF) != SF_FORMAT_PCM_16) {
    ui << "Unsupported audio format in " << filename << "\n";
    return shared_ptr<sample_vec>(NULL);
  }

  if (info.channels != 1) {
    ui << "Didn't find exactly 1 channel while loading stereo audio from "
       << filename << "\n";
    return shared_ptr<sample_vec>(NULL);
  }

  shared_ptr<sample_vec> read_audio(new sample_vec(info.frames));
  sf_count_t frames_read = sf_readf_float(snd_file, read_audio->data(),
                                          info.frames);
  sf_close(snd_file);

  if (frames_read != info.frames) {
    ui << "Error reading audio from " << filename << "\n";
    return shared_ptr<sample_vec>(NULL);
  }

  return read_audio;
}

// TODO eliminate duplicated code between this and load_mono_audio.
shared_ptr<stereo_sample_vec> load_stereo_audio(const std::string &filename) {
  SF_INFO info;
  SNDFILE *snd_file = sf_open(filename.c_str(), SFM_READ, &info);

  if (!snd_file) {
    ui << "Couldn't open " << filename << "\n";
    return shared_ptr<stereo_sample_vec>(NULL);
  }

  if ((info.format & 0xFF) != SF_FORMAT_PCM_16) {
    ui << "Unsupported audio format in " << filename << "\n";
    return shared_ptr<stereo_sample_vec>(NULL);
  }

  if (info.channels != 2) {
    ui << "Didn't find exactly 2 channels while loading stereo audio from "
       << filename << "\n";
    return shared_ptr<stereo_sample_vec>(NULL);
  }

  sample_vec read_audio(info.frames * 2);
  sf_count_t frames_read = sf_readf_float(snd_file, read_audio.data(),
                                          info.frames);
  sf_close(snd_file);

  if (frames_read != info.frames) {
    ui << "Error reading audio from " << filename << "\n";
    return shared_ptr<stereo_sample_vec>(NULL);
  }

  shared_ptr<stereo_sample_vec> audio(new stereo_sample_vec(info.frames));

  copy(read_audio.begin(), read_audio.begin() + info.frames,
       audio->left.begin());

  copy(read_audio.begin() + info.frames + 1, read_audio.end(),
       audio->right.begin());

  return audio;
}
