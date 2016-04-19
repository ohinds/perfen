#!/usr/bin/env python

from aubio import onset
import matplotlib.pyplot as plt
import numpy as np
import os
import sys
from scipy.interpolate import interp1d
from scipy.io import wavfile
from scipy import signal
from scipy import stats

DEBUG_PLOT = False

def smooth(x,window_len=2003,window='flat'):
    """smooth the data using a window with requested size.

    This method is based on the convolution of a scaled window with the signal.
    The signal is prepared by introducing reflected copies of the signal
    (with the window size) in both ends so that transient parts are minimized
    in the begining and end part of the output signal.

    input:
        x: the input signal
        window_len: the dimension of the smoothing window; should be an odd integer
        window: the type of window from 'flat', 'hanning', 'hamming', 'bartlett', 'blackman'
            flat window will produce a moving average smoothing.

    output:
        the smoothed signal

    example:

    t=linspace(-2,2,0.1)
    x=sin(t)+randn(len(t))*0.1
    y=smooth(x)

    see also:

    numpy.hanning, numpy.hamming, numpy.bartlett, numpy.blackman, numpy.convolve
    scipy.signal.lfilter

    TODO: the window parameter could be the window itself if an array instead of a string
    NOTE: length(output) != length(input), to correct this: return y[(window_len/2-1):-(window_len/2)] instead of just y.
    """

    if x.ndim != 1:
        raise ValueError, "smooth only accepts 1 dimension arrays."

    if x.size < window_len:
        raise ValueError, "Input vector needs to be bigger than window size."


    if window_len<3:
        return x


    if not window in ['flat', 'hanning', 'hamming', 'bartlett', 'blackman']:
        raise ValueError, "Window is on of 'flat', 'hanning', 'hamming', 'bartlett', 'blackman'"


    s=np.r_[x[window_len-1:0:-1],x,x[-1:-window_len:-1]]
    #print(len(s))
    if window == 'flat': #moving average
        w=np.ones(window_len,'d')
    else:
        w=eval('np.'+window+'(window_len)')

    y=np.convolve(w/w.sum(),s,mode='valid')
    return y

def get_envelope(buf):
    buf = np.abs(buf)
    buf = smooth(buf)
    return buf

def get_shift_xcor(buf1, buf2):
    buf1 = get_envelope(buf1 / np.max(buf1))
    buf2 = get_envelope(buf2 / np.max(buf2))
    xcorr = np.correlate(buf1, buf2, 'full')
    shift = -(np.argmax(xcorr) - len(xcorr) / 2)

    if (DEBUG_PLOT):
        ax1 = plt.subplot(2, 1, 1)
        ax2 = plt.subplot(2, 1, 2)

        ax1.plot(xcorr)
        ax2.plot(range(len(buf2)) - shift, buf2)
        ax2.plot(buf1)

        plt.show()

    return shift

def get_shift_spec(buf1, rate1, buf2, rate2):
    def bandpass_spec(buf, rate):
        __high_f = 5000
        __low_f = 50
        f, t, Sxx = signal.spectrogram(buf, rate)
        min_f_i = next(x[0] for x in enumerate(f) if x[1] > __low_f)
        max_f_i = next(x[0] for x in enumerate(f) if x[1] > __high_f)
        return f[min_f_i:max_f_i], t, Sxx[:][min_f_i:max_f_i]

    f1, t1, Sxx1 = bandpass_spec(buf1, rate1)
    f2, t2, Sxx2 = bandpass_spec(buf2, rate2)

    # estimate shift
    f_xcorr = np.zeros([2 * t1.shape[0] - 1, f1.shape[0]])
    for f in xrange(len(f1)):
        f_xcorr[:, f] = np.correlate(Sxx1[:][f], Sxx2[:][f], 'full')

    xcorr = np.sum(f_xcorr, 1)

    shift = len(buf1) / t1.shape[0] * -(np.argmax(xcorr) - len(xcorr) / 2)

    if (DEBUG_PLOT):
        ax1 = plt.subplot(4, 1, 1)
        ax2 = plt.subplot(4, 1, 2)
        ax3 = plt.subplot(4, 1, 3)
        ax4 = plt.subplot(4, 1, 4)

        ax1.pcolormesh(t1, f1, Sxx1)
        ax2.pcolormesh(t2, f2, Sxx2)
        ax3.plot(xcorr)
        ax4.plot(range(len(buf2)) - shift, buf2)
        ax4.plot(buf1)

        plt.show()

    return shift

def get_shift_box(buf1, buf2):
    nbuf1 = signal.hilbert(buf1)
    nbuf2 = signal.hilbert(buf2)
    xcorr = np.correlate(nbuf1, nbuf2, 'full')
    shift = -(np.argmax(xcorr) - len(xcorr) / 2)

    if (DEBUG_PLOT):
        ax1 = plt.subplot(2, 1, 1)
        ax1.plot(buf1)
        ax1.plot(nbuf1)

        ax2 = plt.subplot(2, 1, 2)
        ax2.plot(buf2)
        ax2.plot(nbuf2)

        plt.show()

        ax1 = plt.subplot(2, 1, 1)
        ax2 = plt.subplot(2, 1, 2)

        ax1.plot(xcorr)
        ax2.plot(range(len(buf2)) - shift, buf2)
        ax2.plot(buf1)

        plt.show()

    return shift

def main(argv):
    if len(argv) < 3:
        print "usage: %s track1 track2" % argv[0]
        return 1

    rate1, audio1_int = wavfile.read(argv[1])
    rate2, audio2_int = wavfile.read(argv[2])

    audio1 = np.array(audio1_int) / float(max(audio1_int))
    audio2 = np.array(audio2_int) / float(max(audio2_int))

    if rate1 != rate2:
        print "ERROR: files must be of the same sample rate"
        return 2

    window_size = 1.5 * rate1
    abs_power_thresh = 100

    # TODO handle offset starts

    pos1 = 0
    pos2 = 0
    shift = 0
    shifts = []
    rate_damping = 0.5
    rate = 1.0

    out_wav = np.zeros([len(audio1), 2])

    while True:

        print "%d/%d : %d/%d" % (pos1, len(audio1), pos2, len(audio2))

        buf1 = audio1[pos1:pos1 + window_size]
        buf2 = audio2[pos2:pos2 + window_size / rate]

        #buf2 = np.interp(np.arange(0, len(buf2), 1 / rate), np.arange(0, len(buf2)), buf2)

        try:
            buf2 = signal.resample(buf2, window_size)

            out_wav[pos1:pos1 + window_size, 0] = buf1
            out_wav[pos1:pos1 + window_size, 1] = buf2
        except:
            break

        if np.sum(np.abs(buf1)) < abs_power_thresh:
            pos1 += window_size
            pos2 = pos2 + window_size / rate
            continue

        method = 'xcor'
        if method == 'spec':
            shift = get_shift_spec(buf1, rate1, buf2, rate2)
        elif method == 'xcor':
            shift = get_shift_xcor(buf1, buf2)
        elif method == 'box':
            shift = get_shift_box(buf1, buf2)
        else:
            print "error: unknown method %s" % method

        shifts.append(shift)

        print shift

        __min_shift_fit_len = 3
        __max_shift_fit_len = 3
        if len(shifts) < __min_shift_fit_len:
            new_rate = rate - shift / float(len(buf1))
        else:
            n_points = min(__max_shift_fit_len, len(shifts))
            x = range(n_points)
            A = np.vstack([x, np.ones(len(x))]).T
            b = shifts[-n_points:]

            slope, interc = np.linalg.lstsq(A, b)[0]
            new_rate = rate - slope / len(buf1)

        rate = (1 - rate_damping) * rate + rate_damping * new_rate
        print rate

        pos1 += window_size
        pos2 = pos2 + window_size / rate

        if pos1 + window_size > len(audio1) or pos2 + window_size / rate > len(audio2):
            break

    wavfile.write('/tmp/tmp.wav', rate1, out_wav)

if __name__ == "__main__":
    sys.exit(main(sys.argv))
