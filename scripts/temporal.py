#///////////////////////////////////////////////////////////////////////////////
#//               FastNoise - F.A.S.T. Sampling Implementation                //
#//         Copyright (c) 2023 Electronic Arts Inc. All rights reserved.      //
#///////////////////////////////////////////////////////////////////////////////

import sys
import numpy as np
from matplotlib import image
from numpy import pi, sin, cos, modf, sqrt

# TODO: Allow for any spatial filter (box, gauss, binomial)
# Binomial can be hand-coded or iterated uniform filter
from scipy.ndimage.filters import gaussian_filter, uniform_filter
# from scipy.ndimage.filters import gaussian_filter

if len(sys.argv) != 6:
    print(f"Usage: {sys.argv[0]} filename sampleSpace spatialFilter filterParam alpha")
    print("Where sampleSpace is (real|circle|sphere|vector2|vector3|vector4)")
    print("spatialFilter is (uniform|binomial|gauss) with associated parameter (width, n, sigma) filterParam")
    print("alpha is the parameter of the exponential averaging filter")
    exit(1)

# 
filename = sys.argv[1]
sampleSpace = sys.argv[2]
spatialFilter = sys.argv[3]
filterParam = sys.argv[4]
alpha = float(sys.argv[5])

numSamples = 256

img = image.imread(filename)

# Map from [0,1] into [-1,1]
img = (img - 0.5) * 2.0 * 255.0 / 256.0

# Interpret 2D image as a stack of square images
inputShape = img.shape
shape3D = [img.shape[0]//img.shape[1], img.shape[1], img.shape[1], img.shape[2]]
img = np.reshape(img, shape3D)

# 1. Pick a random Heaviside function, and calculate the mask
numFrames = shape3D[0]
variance = np.zeros(numFrames)

for s in range(numSamples):

    filteredResult = np.zeros(img.shape[1] * img.shape[2])

    for k in range(numFrames):

        # Frame is now x,y,d
        frame = np.reshape(img[k], [img.shape[1] * img.shape[2], img.shape[3]])
        mask = np.zeros(frame.shape[0])

        if sampleSpace == "real":
        
            t = 2*(s + np.random.rand()) / numSamples - 1
            for i in range(frame.shape[0]):
                mask[i] = 1.0 if frame[i,0] < t else 0.0

        elif sampleSpace == "circle":
    
            t = 2*(s + np.random.rand()) / numSamples
            for i in range(frame.shape[0]):
                mask[i] = 1.0 if (t + frame[i,0]) % 2.0 < 1.0 else 0.0    
            
        elif sampleSpace == "sphere":
    
            # Uniform sampling of sphere
            phi = 2 * pi * np.random.rand()
            u = 2*np.random.rand()-1
            v = np.array([sqrt(1-u**2) * cos(phi), sqrt(1-u**2) * sin(phi), u])
        
            for i in range(frame.shape[0]):
                mask[i] = 1.0 if np.dot(v, frame[i,0:3]) < 0.0 else 0.0

        elif sampleSpace == "vector2":

            # Generate random Heaviside function
            phi = 2 * pi * np.random.rand()
            offset = sqrt(2) * (2*np.random.rand()-1)
            v = np.array([cos(phi), sin(phi)])

            for i in range(frame.shape[0]):
                mask[i] = 1.0 if np.dot(v, frame[i,0:2]) < offset else 0.0

        elif sampleSpace == "vector3":

            # Generate random Heaviside function
            phi = 2 * pi * np.random.rand()
            u = 2*np.random.rand()-1
            v = np.array([sqrt(1-u**2) * cos(phi), sqrt(1-u**2) * sin(phi), u])
            offset = sqrt(3) * (2*np.random.rand()-1)

            for i in range(frame.shape[0]):
                mask[i] = 1.0 if np.dot(v, frame[i,0:3]) < offset else 0.0

        elif sampleSpace == "vector4":

            # Generate random Heaviside function
            u = np.random.normal(0,1,4)  
            v = u / sqrt(np.sum(u**2))
            offset = 2 * (2*np.random.rand()-1)

            for i in range(frame.shape[0]):
                mask[i] = 1.0 if np.dot(v, frame[i,0:4]) < offset else 0.0

        # Apply spatial filtering to the mask
        # TODO: read params
        mask = np.reshape(mask, [img.shape[1], img.shape[2]])

        if spatialFilter == "box":
            filteredMask = uniform_filter(mask, size=int(filterParam), mode='wrap')
        elif spatialFilter == "gauss":
            filteredMask = gaussian_filter(mask, sigma = float(filterParam), mode='wrap')
        elif spatialFilter == "binomial":
            # Implement binomial filter by repeated size-2 box filters
            filteredMask = uniform_filter(mask, size=2, mode='wrap')
            for i in range(int(filterParam)-1):
                 filteredMask = uniform_filter(filteredMask, size=2, mode='wrap')
        else:
            print(f"Unsupported filter type ({spatialFilter})")
            exit(0)

        filteredMask = np.reshape(filteredMask, img.shape[1] * img.shape[2])


        # Apply temporal filtering
        if k == 0:
            filteredResult = filteredMask
        else:
            #print(filteredResult)
            #print(filteredMask)
            filteredResult = alpha * filteredMask + (1.0 - alpha) * filteredResult

        variance[k] += np.var(filteredResult)

    print(f"{s}/{numSamples}", end="\r")

outputFilename = filename.replace(".png", "_temporal.csv")
np.savetxt(outputFilename, sqrt(variance/numSamples))