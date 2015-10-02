#include "keytracker.h"

#include <algorithm>
#include <numeric>
#include <vector>

#include <sndfile.h>

#include "ui.h"

using std::adjacent_difference;
using std::nth_element;
using std::vector;

namespace {

Ui ui(false);

float find_median_peak_distance(
  const sample_vec& audio, nframes_t start, nframes_t end,
  float peak_threshold) {

  vector<float> peaks;
  nframes_t current_peak_start = -1;
  for (nframes_t frame = start; frame < end; frame++) {
    if (current_peak_start != -1 && audio[frame] < peak_threshold) {
      peaks.push_back(current_peak_start / 2. + frame / 2.);
      current_peak_start = -1;
    }
    else if (current_peak_start == -1 && audio[frame] > peak_threshold) {
      current_peak_start = frame;
    }
  }

  vector<float> peak_diff;
  adjacent_difference(peaks.begin(), peaks.end(), peak_diff.begin());

  nth_element(peak_diff.begin(), peak_diff.begin() + peak_diff.size() / 2,
              peak_diff.end());
  return peak_diff[peak_diff.size() / 2];
}

} // anonymous namespace


KeyTracker::KeyTracker(const std::string& keytrack_file)
  : keytrack_file(keytrack_file)
  , estimated_position(0.)
  , estimated_rate(0.)
  , peak_threshold(0.5)
{}

KeyTracker::~KeyTracker() {}

bool KeyTracker::init() {
  SF_INFO info;
  SNDFILE *snd_file = sf_open(keytrack_file.c_str(), SFM_READ, &info);

  if (!snd_file) {
    ui << "Couldn't open " << keytrack_file << "\n";
    return false;
  }

  if ((info.format & 0xFF) != SF_FORMAT_PCM_16) {
    ui << "Unsupported audio format in " << keytrack_file << "\n";
    return false;
  }

  if (info.channels > 1) {
    ui << "Keytrack audio must be a single channel\n";
    return false;
  }

  sample_vec read_audio(info.frames);
  sf_count_t frames_read = sf_readf_float(snd_file, read_audio.data(),
                                          info.frames);
  sf_close(snd_file);

  if (frames_read != info.frames) {
    ui << "Error reading audio from " << keytrack_file << "\n";
    return false;
  }

  keytrack_audio.resize(info.frames);

  copy(read_audio.begin(), read_audio.end(), keytrack_audio.begin());

  return true;
}

bool KeyTracker::start() {
  return true;
}

bool KeyTracker::addLiveAudio(const sample_vec& audio) {
  unprocessed_audio.insert(unprocessed_audio.end(),
                           audio.begin(), audio.end());
  return true;
}

// update the position and relative rate estimates based on the
// audio added since the last call to this function
bool KeyTracker::update() {
  // find bounds of key audio to be searched
  nframes_t key_start = keytrack_audio.size() * estimated_position;
  nframes_t key_end = key_start + unprocessed_audio.size() / estimated_rate;

  if (key_end >= keytrack_audio.size()) {
    nframes_t new_key_end = keytrack_audio.size() - 1;

    nframes_t unprocessed_end = unprocessed_audio.size() *
      (new_key_end / key_end);

    key_end = keytrack_audio.size() - 1;

    unprocessed_audio.resize(unprocessed_end);
  }

  // search for peaks in key and live audio
  float key_median = find_median_peak_distance(
    keytrack_audio, key_start, key_end, peak_threshold);

  float live_median = find_median_peak_distance(
    unprocessed_audio, 0, unprocessed_audio.size(), peak_threshold);

  estimated_rate = live_median / key_median;

  estimated_position += unprocessed_audio.size() / estimated_rate;

  unprocessed_audio.clear();

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

void KeyTracker::setPeakThreshold(float threshold) {
  peak_threshold = threshold;
}
