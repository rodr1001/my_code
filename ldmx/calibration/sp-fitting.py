import uproot
import numpy as np
import scipy
import matplotlib.pyplot as plt

def scaled_moyal(amplitude, constant, mpv, width):
    return constant*scipy.stats.moyal.pdf(amplitude, loc=mpv, scale=width)

def fit(histogram, drop_zero_bins = True):
    x = histogram.axis().centers()
    y = histogram.values()
    yerr = np.sqrt(histogram.variances())

    if drop_zero_bins:
        x = x[y > 0]
        yerr = yerr[y > 0]
        y = y[y > 0]

    return scipy.optimize.curve_fit(
        scaled_moyal,
        xdata = x,
        ydata = y,
        sigma = yerr,
        absolute_sigma = True
    )


def fit_and_plot(histogram, fit_kw = {}, plt_range = None, **plot_kw):
    opt, cov = fit(histogram, **fit_kw)
    if plt_range is None:
        plt_range = histogram.axis().centers()
    plt.plot(plt_range, scaled_moyal(plt_range, *opt), **plot_kw)



f = uproot.open('hist-no-filtering.root')
h = f['MAC2/MAC2_cell_amplitude_l20_m0_u16_v17']

mplhep.histplot(h, label='Histogram')
fit_and_plot(h, plt_range = np.linspace(0.0,3.0,200), label='Fit')
plt.legend()
plt.show()

###some scipy code, still to work up the csv and see then 
