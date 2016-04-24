#!/usr/bin/env python

import matplotlib.pyplot as plt
import numpy as np
import os
from scipy.interpolate import interp1d
from scipy.io import wavfile
from scipy import signal
from scipy import stats
import subprocess
import sys

DEBUG_PLOT = False

def get_shift(buf1, buf2):
    xcorr = np.correlate(buf1, buf2, 'same')

    # TODO maybe a stronger weighting toward no shift? Also perhaps we
    # can compute this over a shorter time scale?
    weights = range(len(xcorr) / 2) + range(len(xcorr) / 2, 0, -1)

    weighted_xcor = xcorr * weights
    weighted_xcor /= np.max(weighted_xcor)

    shift = -(np.argmax(weighted_xcor) - len(xcorr) / 2)

    if (DEBUG_PLOT):
        ax1 = plt.subplot(4, 1, 1)
        ax2 = plt.subplot(4, 1, 2)
        ax3 = plt.subplot(4, 1, 3)
        ax4 = plt.subplot(4, 1, 4)

        ax1.plot(xcorr)
        ax2.plot(range(len(buf2)) - shift, buf2, alpha=0.5)
        ax2.plot(buf1, alpha=0.5)
        ax3.plot(buf1)
        ax4.plot(buf2)

        plt.show()

    return shift

def main(argv):
    if len(argv) < 3:
        print "usage: %s track1a [track1b ...] track2a [track2b ...] [backing]" % argv[0]
        return 1

    if len(argv) % 2 == 1:
        backing = None
        tracks = argv[1:]
    else:
        backing = argv[-1]
        tracks = argv[1:-1]

    num_tracks_per_song = len(tracks) / 2

    song1_audio = []
    for i in xrange(num_tracks_per_song):
        filename = argv[i + 1]
        print "reading %s" % filename
        sample_rate, audio_int = wavfile.read(filename)
        song1_audio.append(np.array(audio_int, 'float32') / max(audio_int))

    song2_audio = []
    for i in xrange(num_tracks_per_song):
        filename = argv[num_tracks_per_song + i + 1]
        print "reading %s" % filename
        sample_rate, audio_int = wavfile.read(filename)
        song2_audio.append(np.array(audio_int, 'float32') / max(audio_int))

    if backing is not None:
        print "reading %s" % backing
        sample_rate, backing_audio = wavfile.read(backing)

    num_samples = len(song1_audio[0])
    update_samples = int(0.5 * sample_rate)
    window_size = int(1 * sample_rate)

    # TODO handle offset starts

    pos1 = 0
    pos2 = 0
    shift = 0
    shifts = []
    rate_damping = 0.5
    relative_rate = 1.0

    out_wavs = []
    for i in xrange(num_tracks_per_song):
        out_wavs.append(np.zeros([num_samples, 2]))

    back_aligned = np.zeros([num_samples, 2])

    done_proportion = 0.1

    while True:

        print "%d/%d" % (pos1, len(song1_audio[0]))

        if pos1 / float(len(song1_audio[0])) > done_proportion:
            break

        track_shifts = []
        for i in xrange(num_tracks_per_song):
            buf1 = song1_audio[i][pos1:pos1 + window_size]
            buf2 = song2_audio[i][pos2:pos2 + int(window_size / relative_rate)]

            if relative_rate != 1.0:
                buf2 = signal.resample(buf2, window_size)

            track_shifts.append(get_shift(buf1, buf2))

        # TODO estimate confidence
        if len(track_shifts) == 0:
            this_shift = 0
        else:
            this_shift = int(np.mean(track_shifts))

        shifts.append(this_shift)
        print this_shift

        __min_shift_fit_len = 3
        __max_shift_fit_len = 3
        if len(shifts) < __min_shift_fit_len:
            new_relative_rate = relative_rate - this_shift / float(len(buf1))
        else:
            n_points = min(__max_shift_fit_len, len(shifts))
            x = range(n_points)
            A = np.vstack([x, np.ones(len(x))]).T
            b = shifts[-n_points:]

            slope, interc = np.linalg.lstsq(A, b)[0]
            new_relative_rate = relative_rate - slope / len(buf1)

        relative_rate = (1 - rate_damping) * relative_rate + rate_damping * new_relative_rate
        print relative_rate

        # write the shifted output data
        for i in xrange(num_tracks_per_song):
            inds1 = xrange(pos1 + window_size - update_samples, pos1 + window_size)
            out_wavs[i][inds1, 0] = song1_audio[i][inds1]

            inds2 = xrange(pos2 + window_size - update_samples + this_shift,
                           pos2 + window_size + this_shift)
            out_wavs[i][inds1, 1] = song2_audio[i][inds2]

        # hackily stretch the audio
        if backing is not None:
            back_chunk = backing_audio[pos2:int(pos2 + window_size * relative_rate), :]
            wavfile.write('/tmp/back.wav', sample_rate, back_chunk)

            if 0 == subprocess.call(['rubberband',
                                     '-T', '%0.5f' % relative_rate,
                                     '/tmp/back.wav',
                                     '/tmp/back_stretch.wav']):
                _, back_stretch = wavfile.read('/tmp/back_stretch.wav')
                back_stretch = (np.array(back_stretch, 'float32') /
                                np.iinfo(back_stretch.dtype).max)

                inds = xrange(pos1 + window_size - update_samples, pos1 + window_size)
                back_aligned[inds, :] = back_stretch[-update_samples:, :]

                # backfade
                xfade_samples = int(sample_rate / 5.0)
                if pos1 > 0:
                    for sample in xrange(xfade_samples):
                        ind = pos1 + window_size - update_samples - sample - 1

                        ratio = (sample + 1) / xfade_samples
                        for i in range(2):
                            back_aligned[ind, i] = (
                                ratio * back_aligned[ind, i] +
                                (1 - ratio) * back_stretch[-(update_samples + sample + 1), i])

            else:
                print "Error stretching"

        pos1 += update_samples
        pos2 += int(update_samples / relative_rate)

        if (pos1 + window_size > num_samples or
            pos2 + window_size / relative_rate > len(song2_audio[0])):
            break

    for i in xrange(num_tracks_per_song):
        wavfile.write('/tmp/tmp%d.wav' % i, sample_rate, out_wavs[i])

    if backing is not None:
        wavfile.write('/tmp/backing_aligned.wav', sample_rate, back_aligned)

if __name__ == "__main__":
    sys.exit(main(sys.argv))
