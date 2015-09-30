#include "sampler.h"

#include <yaml-cpp/yaml.h>

#include "sample.h"
#include "ui.h"

using std::shared_ptr;
using std::string;

namespace {

typedef std::pair<midi_data_t, Sample*> midi_sample_pair;

Ui ui(false);

}

Sampler::Sampler()
{}

Sampler::~Sampler()
{}

bool Sampler::parseConfig(const string &config_string) {
  YAML::Node node = YAML::Load(config_string);
  for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
    if (!(*it)["file"]) {
      ui << "file is required for each sample\n";
      continue;
    }

    string file((*it)["file"].as<string>());

    float pan = 0.;
    if ((*it)["pan"]) {
      pan = (*it)["pan"].as<float>();
    }

    midi_data_t midi_data = (*it)["midi"].as<midi_data_t>();

    addSample(midi_data, file, pan);
  }
  return true;
}


bool Sampler::processEvent(midi_event_t event, midi_velocity velocity) {

  return true;
}

shared_ptr<stereo_sample_vec> Sampler::getAudio(nframes_t nframes) {

  shared_ptr<stereo_sample_vec> result_audio(new stereo_sample_vec(nframes));

  // loop through playing samples, summing the audio
  for (playing_sample_vec::iterator it = playing_samples.begin();
       it != playing_samples.end(); ) {
    stereo_sample_vec this_audio(nframes);
    if (it->first->getAudioFrames(it->second, nframes, &this_audio)) {
      (*result_audio) += this_audio;
      it->second += nframes;
      ++it;
    }
    else {
      it = playing_samples.erase(it);
    }
  }

  return result_audio;
}

bool Sampler::addSample(midi_data_t data,
                        const string &sample_filename,
                        float pan) {
  if (sample_map.find(data) != sample_map.end()) {
    ui << "refusing to add a sample on a duplicate MIDI event: "
       << data << "\n";
    return false;
  }

  Sample sample(sample_filename, pan);
  if (sample.load()) {
    samples.push_back(sample);
  }

  if (data > 0) {
    sample_map.insert(midi_sample_pair(data, &samples.back()));
  }

  return true;
}
