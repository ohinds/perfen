#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <jack/jack.h>

#include <sndfile.h>

#include <rubberband/RubberBandStretcher.h>

using RubberBand::RubberBandStretcher;
using std::cout;
using std::ifstream;
using std::memcpy;
using std::string;
using std::vector;

typedef jack_default_audio_sample_t sample_t;

typedef std::vector<sample_t> sample_vec;

bool load_wav(const string &filename, sample_vec* audio, SF_INFO* info) {
  SNDFILE *snd_file = sf_open(filename.c_str(), SFM_READ, info);

  if (!snd_file) {
    cout << "Couldn't open " << filename << "\n";
    return false;
  }

  if ((info->format & 0xFF) != SF_FORMAT_PCM_16) {
    cout << "Unsupported audio format in " << filename << "\n";
    return false;
  }

  if (info->channels != 2) {
    cout << "Only stereo files are supported\n";
    return false;
  }

  audio->resize(info->frames * info->channels);

  sf_count_t frames_read = sf_readf_float(snd_file, audio->data(),
                                          info->frames);
  sf_close(snd_file);

  if (frames_read != info->frames) {
    cout << "Error reading audio from " << filename << "\n";
    return false;
  }

  return true;
}

bool load_rates(const string &filename,
                vector<size_t> *ref_samples,
                vector<size_t> *samples,
                vector<float> *rates) {

  ifstream in(filename.c_str());
  if (!in.good()) {
    cout << "Error opening rate file " << filename << "\n";
    return false;
  }

  size_t ref_sample = 0;
  size_t sample = 0;
  float rate = 0.;
  while (true) {

    in >> ref_sample;
    in >> sample;
    in >> rate;

    if (!in.good()) {
      break;
    }

    ref_samples->push_back(ref_sample);
    samples->push_back(sample);
    rates->push_back(rate);
  }

  return true;
}

int main(int argc, char** argv) {

  SF_INFO info;
  sample_vec audio;
  vector<size_t> samples;
  vector<size_t> ref_samples;
  vector<float> rates;
  sample_vec out_audio;

  if (!load_wav(argv[1], &audio, &info)) {
    return 1;
  }

  if (!load_rates(argv[2], &ref_samples, &samples, &rates)) {
    return 2;
  }

  RubberBandStretcher stretcher(
    44100, 2, RubberBandStretcher::OptionProcessRealTime);

  stretcher.setExpectedInputDuration(info.frames);

  float **buffer = new float*[2];
  float **this_out = new float*[2];
  for (size_t i = 1; i < samples.size(); i++) {
    stretcher.setTimeRatio(rates[i]);

    size_t num_samples = samples[i] - samples[i - 1];

    cout << float(i) / samples.size() << " " << rates[i] << " " << num_samples << "\n";

    buffer[0] = new float[num_samples];
    buffer[1] = new float[num_samples];

    for (size_t channel = 0; channel < 2; channel++) {
      for (size_t sample = 0; sample < num_samples; sample++) {
        buffer[channel][sample] = audio[(samples[i - 1] + sample) * 2 + channel];
      }
    }

    stretcher.process(buffer, num_samples, i == samples.size() - 1);

    size_t avail = stretcher.available();

    cout << avail << "\n";

    this_out[0] = new float[avail];
    this_out[1] = new float[avail];
    stretcher.retrieve(this_out, avail);

    for (size_t sample = 0; sample < avail; sample++) {
      for (size_t channel = 0; channel < 2; channel++) {
        out_audio.push_back(this_out[channel][sample]);
      }
    }

    delete [] this_out[0];
    delete [] this_out[1];

    delete [] buffer[0];
    delete [] buffer[1];
  }
  delete [] this_out;
  delete [] buffer;

  cout << "writing file with " << out_audio.size() / 2 << " samples\n";

  // write
  info.frames = out_audio.size() / 2;
  SNDFILE* outfile = sf_open("/tmp/output.wav", SFM_WRITE, &info);
  size_t writen = sf_writef_float(outfile, out_audio.data(), out_audio.size() / 2);
  sf_close(outfile);

  cout << "wrote " << writen << "\n";

  return 0;
}
