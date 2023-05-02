#///////////////////////////////////////////////////////////////////////////////
#//               FastNoise - F.A.S.T. Sampling Implementation                //
#//         Copyright (c) 2023 Electronic Arts Inc. All rights reserved.      //
#///////////////////////////////////////////////////////////////////////////////

import sys
import numpy as np
import matplotlib.pyplot as plt

 #Plot all in gray, then plot single lines over top in red
fig, ax = plt.subplots()
plt.xlabel("Frame number")
plt.ylabel("Average error")
for i in range(2,len(sys.argv)):

    ax.clear()

    for fn in sys.argv[2:]:

        totalError = np.loadtxt(fn)
        ax.plot(totalError, label = fn, color="gainsboro")

    redfn = sys.argv[i]
    totalError = np.loadtxt(redfn)

    plt.yscale("log")
    ax.plot(totalError, label = redfn, color="red")

    #plt.legend(loc='center left', bbox_to_anchor=(1, 0.5))
    plt.savefig(redfn.replace("csv", "png"))

# Scatter plot first frame vs. average frame error
fig, ax = plt.subplots()
plt.xlabel("First frame error")
plt.ylabel("Average frame error")
for fn in sys.argv[2:]:
    totalError = np.loadtxt(fn)
    first = totalError[0]
    average = np.sqrt(np.sum(totalError**2))
    last = totalError[-1]

    if "whitenoise" in fn:
        continue

    plt.scatter(first, average, marker = 's' if "product" in fn else "P", label = fn)


lgd = plt.legend(loc='center left', bbox_to_anchor=(1, 0.5))
plt.savefig(sys.argv[1], bbox_extra_artists=(lgd,), bbox_inches='tight')
