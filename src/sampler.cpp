#include "sampler.h"

#include <yaml-cpp/yaml.h>

using std::shared_ptr;
using std::string;

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

    Sample *sample = new Sample(file);
    samples.push_back(sample);

    if ((*it)["key"]) {
      key_sample_map[(*it)["key"].as<char>()] = sample;
    }

    if (use_midi && (*it)["midi"]) {
      midi_sample_map[(*it)["midi"].as<int>()] = sample;
    }

    if ((*it)["pan"]) {
      sample->setPan((*it)["pan"].as<float>());
    }
  }
  return true;
}


bool Sampler::processEvent(midi_event_t event, midi_velocity velocity) {

  return true;
}

shared_ptr<sample_vec> Sampler::getAudio(nframes_t nframes) {

  shared_ptr<sample_vec> result_audio(new sample_vec(nframes, 0));

  return result_audio;
}

bool Sampler::addSample(midi_data_t data, const string &sample_filename) {
  if (sample_map.find(data) != sample_map.end()) {
    // TODO print an error message
    return false;
  }

  // TODO load audio file

  return true;
}
