#include "keytracked_sequencer.h"

#include <yaml-cpp/yaml.h>

#include "ui.h"

using std::shared_ptr;
using std::string;

namespace {
  Ui ui(false);
}

KeytrackedSequencer::KeytrackedSequencer()
  : position(0.)
  , relative_rate(1.)
{}

KeytrackedSequencer::~KeytrackedSequencer() {}

bool KeytrackedSequencer::parseConfig(const string& config_string) {
  YAML::Node node = YAML::Load(config_string);
  for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
    string sample_name = it->first.as<string>();
    ui << sample_name << "\n";

    YAML::Node sample_data = it->second;

    if (!sample_data["file"] || !sample_data["position"]) {
      ui << "file and position are required for each sample\n";
      continue;
    }

    string file(sample_data["file"].as<string>());
    float position = sample_data["position"].as<float>();

    float pan = 0.;
    if (sample_data["pan"]) {
      pan = sample_data["pan"].as<float>();
    }

    Sample sample(file, pan);
    if (!sample.load()) {
      ui << "error loading sample " << file << " at position "
         << position << ", skipping\n";
      continue;
    }

    sequence.push_back(positioned_sample(position, sample));
  }
  return true;
}

bool KeytrackedSequencer::setPosition(float new_position) {
  position = new_position;

  // TODO stretch some audio
}

bool KeytrackedSequencer::setRelativeRate(float new_relative_rate) {
  relative_rate = new_relative_rate;

  // TODO stretch some audio
}

shared_ptr<stereo_sample_vec> KeytrackedSequencer::getAudio(nframes_t nframes) const {

  // TODO use stretched audio

  shared_ptr<stereo_sample_vec> audio_frames(new stereo_sample_vec(nframes));

  return audio_frames;
 }
