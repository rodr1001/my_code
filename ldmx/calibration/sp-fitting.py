###scipy code now with laundau 
###  originally ran in a jupyter lab

import uproot
import numpy as np
import scipy
import matplotlib.pyplot as plt

# landau is provided by https://github.com/SengerM/landaupy
from landaupy import landau

def scaled_landau(amplitude, constant, mpv, width):
    return constant*landau.pdf(amplitude, x_mpv=mpv, xi=width)

def fit(histogram, drop_zero_bins = True):
    x = histogram.axis().centers()
    y = histogram.values()
    yerr = np.sqrt(histogram.variances())

    if drop_zero_bins:
        x = x[y > 0]
        yerr = yerr[y > 0]
        y = y[y > 0]

    return scipy.optimize.curve_fit(
        scaled_landau,
        xdata = x,
        ydata = y,
        sigma = yerr,
        absolute_sigma = True
    )


def fit_and_plot(histogram, fit_kw = {}, plt_range = None, **plot_kw):
    opt, cov = fit(histogram, **fit_kw)
    if plt_range is None:
        plt_range = histogram.axis().centers()
    mplhep.histplot(histogram, label='Histogram')
    plt.plot(plt_range, scaled_landau(plt_range, *opt), label='Fit', **plot_kw)
    plt.legend()
    return opt, cov

f = uproot.open('hist-no-filtering.root')

opt, cov = fit_and_plot(
    f['MAC2/MAC2_cell_amplitude_l0_m0_u16_v12'],
    plt_range=np.linspace(0,3,200)
)
plt.axvline(0.26, color='gray')
plt.show()

print('[Constant x_MPV Sigma]')
print(opt)
print(np.sqrt(np.diag(cov)))


