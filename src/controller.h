// class to control audio and midi flow.

#pragma once

#include <string>

#include "types.h"

class Controller {

 public:

  Controller(const std::string& configfile);

  ~Controller();

  bool init();

  bool run();

 private:

  bool parseConfig();

  static int processCallback(nframes_t nframes, void* arg);

  int process(nframes_t nframes);

  std::string configfile;

};
