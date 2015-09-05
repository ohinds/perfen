#include "sampler.h"

#include <yaml-cpp/yaml.h>

#include "sample.h"
#include "ui.h"

using std::shared_ptr;
using std::string;

namespace {
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

    // if ((*it)["key"]) {
    //   key_sample_map[(*it)["key"].as<char>()] = sample;
    // }

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

bool Sampler::addSample(midi_data_t data,
                        const string &sample_filename,
                        float pan) {
  if (sample_map.find(data) != sample_map.end()) {
    // TODO print an error message
    return false;
  }

  // TODO load audio file

  return true;
}
