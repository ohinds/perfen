// utility functions for perfen

#include <memory>
#include <string>

#include "types.h"

// Query the number of audio channels in an audio file. Returns 0 if
// there was an error opening the file.
size_t get_num_channels(const std::string &file);

// Load audio samples from a mono audio file. Returns a NULL pointer
// if audio could not be loaded.
std::shared_ptr<sample_vec> load_mono_audio(const std::string &file);

// Load audio samples from a stereo audio file. Returns a NULL pointer
// if audio could not be loaded.
std::shared_ptr<stereo_sample_vec> load_stereo_audio(const std::string &file);
