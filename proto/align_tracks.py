#!/usr/bin/env python

from aubio import onset, pitch
import matplotlib.pyplot as plt
import numpy as np
import os
import sys
from scipy.io import wavfile
import scipy.signal as signal

DEBUG_PLOT = False

class Tracker(object):

    def __init__(self, wav1, wav2):

        rate1, audio1_int = wavfile.read(wav1)
        rate2, audio2_int = wavfile.read(wav2)

        self.audio1 = np.array(audio1_int) / float(max(audio1_int))
        self.audio2 = np.array(audio2_int) / float(max(audio2_int))

        if rate1 != rate2:
            raise ValueError("files must be of the same sample rate")

        self.sample_rate = rate1
        self.window_size = self.sample_rate
        self.aubio_fft_win = self.window_size / 10
        self.aubio_hop_size = self.aubio_fft_win / 2

        self.pos1 = 0
        self.pos2 = 0

        self.relative_rate = 1.0
        self.rate_damping = 0.5

        self.power_ignore_threshold = 100

        self.out_wav = np.zeros([len(self.audio1), 2])

        self.shifts = []

        self.pitch_estimator1 = pitch("yinfft",
                                      self.aubio_fft_win,
                                      self.aubio_hop_size,
                                      self.sample_rate)
        self.pitch_estimator2 = pitch("yinfft",
                                      self.aubio_fft_win,
                                      self.aubio_hop_size,
                                      self.sample_rate)

        self.onset_estimator1 = onset("default",
                                      self.aubio_fft_win,
                                      self.aubio_hop_size,
                                      self.sample_rate)
        self.onset_estimator2 = onset("default",
                                      self.aubio_fft_win,
                                      self.aubio_hop_size,
                                      self.sample_rate)


    def estimate_shift(self, buf1, buf2):

        def get_onsets(estimator, buf):
            onsets = []

            for ind in xrange(0, len(buf), self.aubio_hop_size):
                estimator(buf[ind:ind+self.aubio_hop_size])
                onsets.append(estimator.get_last())

            return onsets

        def get_pitches(estimator, buf):
            pitches = []

            for ind in xrange(0, len(buf), self.aubio_hop_size):
                pitch = estimator(buf[ind:ind+self.aubio_hop_size])[0]
                confidence = estimator.get_confidence()
                pitches.append([pitch, confidence])

            return pitches


        onsets1 = get_onsets(self.onset_estimator1, buf1)
        onsets2 = get_onsets(self.onset_estimator2, buf2)

        pitches1 = get_pitches(self.pitch_estimator1, buf1)
        pitches2 = get_pitches(self.pitch_estimator2, buf2)


        import ipdb; ipdb.set_trace()


        return shift

    def run(self):

        while True:
            print "%0.2f%% : %0.2f%%" % (
                100 * self.pos1 / float(len(self.audio1)),
                100 * self.pos2 / float(len(self.audio2)))

            buf1 = self.audio1[self.pos1:self.pos1 + self.window_size]
            buf2 = self.audio2[
                self.pos2:self.pos2 + self.window_size / self.relative_rate]
            buf2 = signal.resample(buf2, self.window_size)

            self.out_wav[self.pos1:self.pos1 + self.window_size, 0] = buf1
            self.out_wav[self.pos1:self.pos1 + self.window_size, 1] = buf2

            if (np.sum(np.abs(buf1)) < self.power_ignore_threshold or
                np.sum(np.abs(buf2)) < self.power_ignore_threshold):
                self.pos1 += self.window_size
                self.pos2 += self.window_size / self.relative_rate
                continue

            ## MAGIC!
            shift = self.estimate_shift(buf1, buf2)

            shifts.append(shift)

            print shift

            new_rate = shift / float(len(buf1))
            self.relative_rate = ((1 - self.rate_damping) * self.relative_rate +
                                  self.rate_damping * new_rate)
            print rate

            self.pos1 += self.window_size
            self.pos2 = self.pos2 + self.window_size / self.relative_rate

            # termination
            if (self.pos1 + self.window_size > len(self.audio1) or
                self.pos2 + self.window_size / self.relative_rate > len(self.audio2)):
                break

        wavfile.write('/tmp/tmp.wav', rate1, out_wav)

def main(argv):
    if len(argv) < 3:
        print "usage: %s track1 track2" % argv[0]
        return 1

    tracker = Tracker(argv[1], argv[2])
    tracker.run()

if __name__ == "__main__":
    sys.exit(main(sys.argv))
