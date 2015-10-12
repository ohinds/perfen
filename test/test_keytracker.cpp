#include "keytracker.h"

#include <memory>
#include <string>

#include "gtest/gtest.h"

#include "util.h"

using std::shared_ptr;
using std::string;

namespace {

class KeyTrackerTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  KeyTrackerTest() {
    // You can do set-up work for each test here.
  }

  virtual ~KeyTrackerTest() {
    // You can do clean-up work that doesn't throw exceptions here.
  }

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:

  virtual void SetUp() {
    // Code here will be called immediately after the constructor (right
    // before each test).
  }

  virtual void TearDown() {
    // Code here will be called immediately after each test (right
    // before the destructor).
  }

  // Objects declared here can be used by all tests in the test case for Foo.
};

// Test that the keytracker tracks an identity key properly
TEST_F(KeyTrackerTest, IdentityTest) {
  const string keytrack_file = "../test/data/keytrack.wav";
  const string livetrack_file = "../test/data/keytrack.wav";

  KeyTracker tracker;
  ASSERT_TRUE(tracker.init(keytrack_file));

  // load the live audio
  shared_ptr<sample_vec> live_audio = load_mono_audio(livetrack_file);
  ASSERT_FALSE(!live_audio);

}

}  // namespace
