#include "controller.h"

#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>

#include <jack/jack.h>
#include <jack/midiport.h>

#include <yaml-cpp/yaml.h>

#include "keytracked_sequencer.h"
#include "keytracker.h"
#include "sampler.h"
#include "ui.h"

using std::chrono::milliseconds;
using std::string;
using std::this_thread::sleep_for;
using std::vector;

namespace {

jack_client_t *client;
vector<jack_port_t*> input_audio_ports;
vector<jack_port_t*> input_midi_ports;
vector<jack_port_t*> output_audio_ports;

KeyTracker keytracker;
KeytrackedSequencer keytracked_sequencer;
Sampler sampler;
Ui ui(false);

} // anonymous namespace

Controller::Controller(const string& config_file)
  : config_file(config_file) {
}

Controller::~Controller() {
  jack_client_close(client);
}

bool Controller::init() {
  // setup the jack client
  const char *client_name = "perfen";
  const char *server_name = NULL;
  jack_options_t options = JackNullOption;
  jack_status_t status;

  client = jack_client_open(client_name, options, &status, server_name);
  if (client == NULL) {
    ui << "jack_client_open() failed, status = " << status << "\n";
    if (status & JackServerFailed) {
      ui << "Unable to connect to JACK server\n";
    }
    return false;
  }
  if (status & JackServerStarted) {
    ui << "JACK server started\n";
  }
  if (status & JackNameNotUnique) {
    client_name = jack_get_client_name(client);
    ui << "Jackl client '" << client_name << "' assigned\n";
  }

  jack_set_process_callback(client, processCallback, this);

  if (!parseConfig()){
    ui << "Parsing config file failed\n";
    return false;
  }

  return true;
}

bool Controller::run() {
  if (jack_activate(client)) {
    ui << "cannot activate client\n";
    return false;
  }

  if (jack_connect(client, "system:capture_1", "perfen:input_L")) {
    ui << "cannot connect input audio port 0\n";
    return false;
  }

  if (jack_connect(client, "system:capture_2", "perfen:input_R")) {
    ui << "cannot connect input audio port 1\n";
    return false;
  }

  if (jack_connect(client, "perfen:output_L", "system:playback_1")) {
    ui << "cannot connect output audio port 0\n";
    return false;
  }

  if (jack_connect(client, "perfen:output_R", "system:playback_2")) {
    ui << "cannot connect output audio port 1\n";
    return false;
  }

  while (true) {
    sleep_for(milliseconds(100));
  }

  return true;
}

bool Controller::parseConfig() {
  YAML::Node node = YAML::LoadFile(config_file);
  if (node["samples"]) {
    ui << YAML::Dump(node["samples"]) << "\n";
    sampler.parseConfig(YAML::Dump(node["samples"]));
  }

  // TODO read config and register ports, for now just register a
  // stereo input audio port, an input MIDI port, and a stereo output
  // port
  input_audio_ports.push_back(
    jack_port_register(
      client, "input_L", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0));
  input_audio_ports.push_back(
    jack_port_register(
      client, "input_R", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0));
  input_midi_ports.push_back(
    jack_port_register(
      client, "input_MIDI", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0));

  output_audio_ports.push_back(
    jack_port_register(
      client, "output_L", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0));
  output_audio_ports.push_back(
    jack_port_register(
      client, "output_R", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0));

  return true;
}

int Controller::processCallback(nframes_t nframes, void* arg) {
  Controller* controller = static_cast<Controller*>(arg);
  return controller->process(nframes);
}

int Controller::process(nframes_t nframes) {

  // get the input sample buffers
  sample_t *in = static_cast<sample_t*>(
    jack_port_get_buffer(input_audio_ports[0], nframes));
  sample_vec in_l(in, in + nframes);

  in = static_cast<sample_t*>(
    jack_port_get_buffer(input_audio_ports[1], nframes));
  sample_vec in_r(in, in + nframes);

  // float sum = 0.;
  // for (sample_vec::iterator it = in_l.begin(); it != in_l.end(); ++it) {
  //   sum += *it;
  // }
  // ui << "sum is " << sum << "\n";

  // process MIDI events
  // TODO incomplete?
  void* midi_in_buffer = jack_port_get_buffer(input_midi_ports[0], nframes);
  nframes_t event_count = jack_midi_get_event_count(midi_in_buffer);
  if (event_count > 1) {
    ui << "Processing " << event_count << " MIDI events\n";
    midi_event_t in_event;
    for(int i = 0; i < event_count; i++) {
      jack_midi_event_get(&in_event, midi_in_buffer, i);
      ui << "MIDI event " << i << " at time " << in_event.time
         << ". 1st byte is " << *(in_event.buffer) << "\n";
    }
  }

  // TODO route the MIDI and audio

  // retreive the output buffers
  sample_t *out_l = static_cast<sample_t*>(
    jack_port_get_buffer(output_audio_ports[0], nframes));

  sample_t *out_r = static_cast<sample_t*>(
    jack_port_get_buffer(output_audio_ports[1], nframes));

  // write to the output buffers
  memcpy(out_l, in_l.data(), sizeof(sample_t) * nframes);
  memcpy(out_r, in_r.data(), sizeof(sample_t) * nframes);

  return 0;
}

int main(int argc, char** args) {
  if (argc < 2) {
    std::cerr << "usage: " << args[0] << " <config file>\n";
    return 1;
  }

  Controller controller(args[1]);

  if (!controller.init()) {
    std::cerr << "Controller init failed\n";
    return 2;
  }

  if (!controller.run()) {
    std::cerr << "Controller run failed\n";
    return 3;
  }

  return 0;
}
