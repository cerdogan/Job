# @file rejection.py
# @author Can Erdogan
# @date 2015-08-04
# @brief Implementation of rejection sampling to sample from a Rayleigh distribution where
# a Gaussian distribution is used to "over-estimate" the Rayleigh.

import pylab as pl
import numpy as np
import math 
from random import uniform

fig = pl.figure()

# Draw the Rayleigh distribution
X = np.linspace(0, 10, 1000, endpoint=True)
sigma_R = 2.0
R = (X/(sigma_R*sigma_R))*np.exp(-X*X/(2*sigma_R*sigma_R))
pl.plot(X, R)

# Draw the Gaussian distribution
M = 2.5
X = np.linspace(-7, 13, 1000, endpoint=True)
sigma_G = 3.0
mu = 3.0
G = (1/(sigma_G*np.sqrt(2*np.pi)))*np.exp(-((X-mu)*(X-mu))/(2*sigma_G*sigma_G))
pl.plot(X, M*G)

# Rejection sampling
samples = 0
f = open('bla', 'w+')
vals = []
vals_vx = []
while samples < 10000:

	# Sample the Gaussian with the Box-Muller
	u1 = uniform(0, 1)
	u2 = uniform(0, 1)
	r = np.sqrt(-2*np.log(u1))
	th = 2*np.pi*u2
	x = sigma_G * (r * np.cos(th)) + mu

	# Sample the vertical
	vx_max = M * (1/(sigma_G*np.sqrt(2*np.pi)))*np.exp(-((x-mu)*(x-mu))/(2*sigma_G*sigma_G))
	vx = uniform(0, vx_max)
	# print "%f, %f" % (x, vx)
  
  # Compare with the limit
	vx_lim = (x/(sigma_R*sigma_R))*np.exp(-x*x/(2*sigma_R*sigma_R))
	if vx < vx_lim:
		pl.plot(x, vx, markersize=2, marker='o', color='green')
		vals.append(x)
		vals_vx.append(vx)
		samples += 1
	else:
		 pl.plot(x, vx, markersize=2, marker='o', color='red')

# Draw the histogram
hist, bins = np.histogram(vals, 20)
bins_mid = []
bars = []
for i in range(0,20):
	# bins_mid.append((bins[i]+bins[i+1])/2)
	bins_mid.append(0.8*float(bins[i])+0.2*float(bins[i+1]))
	bars.append(float(hist[i]) / np.max(hist) * np.max(vals_vx))
#y = (bins/(sigma_R*sigma_R))*np.exp(-bins*bins/(2*sigma_R*sigma_R))
pl.bar(bins_mid, bars, width=0.2) #, 'k--', linewidth=1.5)

fig.suptitle('Rejection Sampling from a Reyleigh Distribution', fontsize=30)
pl.show()
