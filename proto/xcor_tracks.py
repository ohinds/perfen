#!/usr/bin/env python

import matplotlib.pyplot as plt
import numpy as np
import os
import sys
from scipy.io import wavfile
from scipy import signal

def smooth(x,window_len=1001,window='hanning'):
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

    window_size = rate1
    abs_power_thresh = 100

    # TODO handle offset starts

    pos1 = 0
    pos2 = 0
    shift = 0

    debug_plot = True
    while True:

        buf1 = smooth(np.power(audio1[pos1:pos1 + window_size], 2))
        buf2 = smooth(np.power(audio2[pos2:pos2 + window_size], 2))

        if np.sum(np.abs(buf1)) < abs_power_thresh:
            pos1 += window_size
            pos2 = pos1 + shift
            continue

        xcorr = np.correlate(buf1, buf2, 'full')

        shift = -(np.argmax(xcorr) - len(xcorr) / 2)

        if (debug_plot):
            ax1 = plt.subplot(2, 1, 1)
            ax2 = plt.subplot(2, 1, 2)

            print shift

            ax1.plot(buf1)
            ax1.plot(range(len(buf2)) - shift, buf2)
            ax1.plot(buf2)
            ax2.plot(xcorr)

            plt.show()

        pos1 += window_size
        pos2 = pos1 + shift

        if pos1 + window_size > len(audio1) or pos2 + window_size > len(audio2):
            break


if __name__ == "__main__":
    sys.exit(main(sys.argv))
