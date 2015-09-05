#include "sample.h"

#include <iostream>
#include <string>

#include <sndfile.h>

using std::string;

Sample::Sample(const string &filename, float pan)
  : filename(filename)
{
  SF_INFO info;
  SNDFILE *snd_file = sf_open(filename.c_str(), SFM_READ, &info);

  if (!snd_file) {
    //cerr << "Couldn't open " << filename << endl;
    return;
  }

  if ((info.format & 0xFF) != SF_FORMAT_PCM_16) {
    //cerr << "Unsupported audio format in " << filename << endl;
    return;
  }

  audio.resize(info.channels);
  for (int c = 0; c < audio.size(); c++) {
    audio[c].resize(info.frames);

    // TODO read the channel audio
  }

  // sf_readf_short(snd_file, frames, info.frames);
  // sf_close(snd_file);
}

Sample::~Sample() {
}
