#include <iostream>

#include "controller.h"

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
