#include "keytracker.h"

#include <string>

#include "gtest/gtest.h"
#include "util.h"


using std::shared_ptr;
using std::string;

namespace {

class KeyTrackerTest : public ::testing::Test {};

// Test that the keytracker tracks an identity key properly
TEST_F(KeyTrackerTest, IdentityTest) {
  const string keytrack_file = "../test/data/keytrack.wav";
  const string livetrack_file = "../test/data/keytrack.wav";

  KeyTracker tracker;
  ASSERT_TRUE(tracker.init(keytrack_file));

  // load the live audio
  shared_ptr<sample_vec> live_audio = load_mono_audio(livetrack_file);
  ASSERT_FALSE(!live_audio);

  const float CHUNK_TIME_S = 1.;
  const nframes_t CHUNK_FRAMES = CHUNK_TIME_S * 44100;
  for (nframes_t frame = CHUNK_FRAMES, sent_frames = 0;
       frame < live_audio->size(); frame += CHUNK_FRAMES) {
    sample_vec chunk(live_audio->begin() + frame - CHUNK_FRAMES,
                     live_audio->begin() + frame - 1);
    tracker.addLiveAudio(chunk);
    tracker.update();
    sent_frames += chunk.size();

    EXPECT_EQ(1., tracker.getEstimatedRelativeRate());
    EXPECT_FLOAT_EQ(static_cast<float>(sent_frames) / live_audio->size(),
              tracker.getEstimatedPosition());
  }


}

}  // namespace
