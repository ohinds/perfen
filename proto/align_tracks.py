#!/usr/bin/env python

from aubio import onset, pitch, source
import math
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import os
import sys
from scipy.io import wavfile
import scipy.signal as signal

DEBUG_PLOT = True
matplotlib.interactive(False)

class DebugPlot(object):

    def __init__(self, num_ax=1):
        self.fig = plt.figure()
        self.ax = []
        for ax in xrange(num_ax):
            self.ax.append(self.fig.add_subplot(num_ax, 1, ax + 1))

    def show(self):
        plt.show()

    def waveform(self, wav, x=None, ax=0):
        if x is None:
            x = xrange(len(wav))
        self.ax[ax].plot(wav)

    def onsets(self, onsets, ax=0):
        y_min, y_max = self.ax[ax].get_ylim()

        for on in onsets:
            self.ax[ax].plot([on, on], [y_min, y_max], color='r')

class Tracker(object):

    def __init__(self, wav1, wav2):

        rate1, audio1_int = wavfile.read(wav1)
        rate2, audio2_int = wavfile.read(wav2)

        self.audio1 = np.array(audio1_int, 'float32') / max(audio1_int)
        self.audio2 = np.array(audio2_int, 'float32') / max(audio2_int)

        if rate1 != rate2:
            raise ValueError("files must be of the same sample rate")

        self.sample_rate = rate1
        self.window_size = 2 * 51200
        self.aubio_fft_win = 512
        self.aubio_hop_size = self.aubio_fft_win / 2

        self.pos1 = 0
        self.pos2 = 0

        self.relative_rate = 1.0
        self.rate_damping = 0.5

        self.power_ignore_threshold = 100

        self.out_wav = np.zeros([len(self.audio1), 2])

        self.shifts = []

        self.pitch_estimator1 = pitch('yinfft',
                                      self.aubio_fft_win,
                                      self.aubio_hop_size,
                                      self.sample_rate)

        self.pitch_estimator2 = pitch('yinfft',
                                      self.aubio_fft_win,
                                      self.aubio_hop_size,
                                      self.sample_rate)

        self.onset_estimator1 = onset('default',
                                      self.aubio_fft_win,
                                      self.aubio_hop_size,
                                      self.sample_rate)
        #self.onset_estimator1.set_threshold(0.8)
        #self.onset_estimator1.set_silence(0.05)
        #self.onset_estimator1.set_minioi(10000)

        self.onset_estimator2 = onset('default',
                                      self.aubio_fft_win,
                                      self.aubio_hop_size,
                                      self.sample_rate)
        #self.onset_estimator2.set_threshold(0.8)
        #self.onset_estimator2.set_silence(1)
        #self.onset_estimator2.set_minioi(10000)


    def estimate_shift(self, buf1, offset1, buf2, offset2):

        def get_onsets(estimator, buf, offset):
            onsets = []

            for ind in xrange(0, len(buf), self.aubio_hop_size):
                if estimator(buf[ind:ind+self.aubio_hop_size]):
                    onsets.append(estimator.get_last() - offset)

            print onsets
            return onsets

        def get_pitches(estimator, buf):
            pitches = []

            for ind in xrange(0, len(buf), self.aubio_hop_size):
                pitch = estimator(buf[ind:ind+self.aubio_hop_size])[0]
                confidence = estimator.get_confidence()

                if not math.isnan(confidence) and confidence > 0.5:
                    pitches.append([pitch, confidence])

            return pitches

        onsets1 = get_onsets(self.onset_estimator1, buf1, offset1)
        onsets2 = get_onsets(self.onset_estimator2, buf2, offset2)

        if DEBUG_PLOT:
            dp = DebugPlot(2)
            dp.waveform(buf1, ax=0)

            if len(onsets1) > 0:
                dp.onsets(onsets1, ax=0)

            dp.waveform(buf2, ax=1)

            if len(onsets2) > 0:
                dp.onsets(onsets2, ax=1)

            dp.show()

        pitches1 = get_pitches(self.pitch_estimator1, buf1)
        pitches2 = get_pitches(self.pitch_estimator2, buf2)

        shift = 0
        return shift

    def run(self):

        while True:
            print "%0.2f%% : %0.2f%%" % (
                100 * self.pos1 / float(len(self.audio1)),
                100 * self.pos2 / float(len(self.audio2)))

            buf1 = self.audio1[self.pos1:self.pos1 + self.window_size]
            buf2 = self.audio2[
                self.pos2:self.pos2 + int(self.window_size / self.relative_rate)]
            buf2 = np.array(signal.resample(buf2, self.window_size), 'float32')

            try:
                self.out_wav[self.pos1:self.pos1 + self.window_size, 0] = buf1
                self.out_wav[self.pos1:self.pos1 + self.window_size, 1] = buf2
            except:
                break

            # if (np.sum(np.abs(buf1)) < self.power_ignore_threshold or
            #     np.sum(np.abs(buf2)) < self.power_ignore_threshold):
            #     self.pos1 += self.window_size
            #     self.pos2 += self.window_size / self.relative_rate
            #     continue

            ## MAGIC!
            shift = self.estimate_shift(buf1, self.pos1, buf2, self.pos2)

            #shifts.append(shift)

            print shift

            new_rate = self.relative_rate - shift / float(len(buf1))
            self.relative_rate = ((1 - self.rate_damping) * self.relative_rate +
                                  self.rate_damping * new_rate)
            print self.relative_rate

            self.pos1 += self.window_size
            self.pos2 = self.pos2 + self.window_size / self.relative_rate

            # termination
            if (self.pos1 + self.window_size > len(self.audio1) or
                self.pos2 + self.window_size / self.relative_rate > len(self.audio2)):
                break

        wavfile.write('/tmp/tmp.wav', self.sample_rate, self.out_wav)
        print "wrote wav file"

def main(argv):
    if len(argv) < 3:
        print "usage: %s track1 track2" % argv[0]
        return 1

    tracker = Tracker(argv[1], argv[2])
    tracker.run()

if __name__ == "__main__":
    sys.exit(main(sys.argv))
